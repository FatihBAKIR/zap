#include <bb/registry.hpp>
#include <boost/dll.hpp>
#include <iostream>

#include <flatbuffers/flatbuffers.h>
#include <vector>
#include <bb/auth.hpp>

#include <boost/asio.hpp>

#include <fbs/req_generated.h>

#include <boost/process.hpp>

#include <spdlog/spdlog.h>
#include "spdlog/sinks/stdout_color_sinks.h"

#include <thread>
#include <bb/handler.hpp>
#include <bb/dynamic.hpp>
#include <nlohmann/json.hpp>
#include <bb/json_auth.hpp>

using boost::asio::ip::udp;

struct module_info
{
    explicit module_info(boost::dll::shared_library&& l) : lib(std::move(l))
    {
        reg = &lib.get<zap::dynamic_registry>("registry");
    }

    zap::dynamic_registry* reg;
private:
    boost::dll::shared_library lib;
};

struct iauth
{
    virtual bb::client_id_t authenticate(const std::string&) = 0;
    virtual ~iauth() = default;
};

struct rem_auth : iauth
{
    bb::client_id_t authenticate(const std::string &string) override {
        flatbuffers::FlatBufferBuilder builder(256);
        auto tok_off = builder.CreateString("");
        auto body_off = builder.CreateVector<uint8_t>((const uint8_t *)string.data(), string.size());
        auto handler_off = builder.CreateString("auth");
        auto req = zap::cloud::CreateRequest(builder, tok_off, handler_off, body_off);
        builder.Finish(req);

        std::array<uint8_t, 512> buf;
        auto r = sock.send_to(boost::asio::buffer(builder.GetBufferPointer(), builder.GetSize()), ep);
        udp::endpoint e;

        boost::system::error_code ec;
        auto len = sock.receive_from(boost::asio::buffer(buf), e, 0, ec);

        if (len == 0 || ec || e != ep)
        {
            return {};
        }

        if (buf[0] != '{')
        {
            return {};
        }

        auto j = nlohmann::json::parse(buf.data(), buf.data() + len);
        return bb::deserialize(j);
    }

    rem_auth(boost::asio::io_context& ioc, const udp::endpoint& e) : sock(ioc), ep(e) {
        sock.open(udp::v4());
    }

private:
    udp::socket sock;
    udp::endpoint ep;
};

struct null_authenticator : iauth
{
    bb::client_id_t authenticate(const std::string &string) override {
        return {};
    }
};

iauth* auth = new null_authenticator;

class server
{
public:
    server(boost::asio::io_context& io_context)
            : socket_(io_context, udp::endpoint(udp::v4(), 0))
    {
        auto req_log = spdlog::stderr_color_mt("req-log");
        do_receive();
    }

    server(boost::asio::io_context& io_context, uint16_t port)
            : socket_(io_context, udp::endpoint(udp::v4(), port))
    {
        auto req_log = spdlog::stderr_color_mt("req-log");
        do_receive();
    }

    void load_module(std::string_view ns, boost::dll::shared_library&& lib)
    {
        auto [it, ins] = mods.emplace(ns, std::move(lib));
        if (ins); // do what?
    }

    uint16_t get_port() const
    {
        return socket_.local_endpoint().port();
    }

private:
    void handle_packet(tos::span<const uint8_t> packet, const udp::endpoint& from)
    {
        auto ver = flatbuffers::Verifier(packet.data(), packet.size());
        bool ok = zap::cloud::VerifyRequestBuffer(ver);

        auto req = flatbuffers::GetRoot<zap::cloud::Request>(packet.data());

        auto remote_addr = from.address().to_string();
        auto remote_port = from.port();

        auto req_log = spdlog::get("req-log");
        if (!ok)
        {
            req_log->error("Received garbage from {}:{}", remote_addr, remote_port);
            return;
        }

        auto body = tos::span<const uint8_t>(req->body()->data(), req->body()->size());

        auto token = req->token()->str();

        auto handler = req->handler()->str();

        req_log->info(
                "Got request on \"{}\" with body size {} from {}:{}",
                handler, body.size(), remote_addr, remote_port);

        try
        {
            auto id = auth->authenticate(token);

            auto fun_log = spdlog::get(handler);

            zap::call_info ci;
            ci.client = id;
            ci.log = fun_log ? fun_log : spdlog::stdout_color_mt(handler);

            auto dot_pos = handler.find('.');

            auto ns = handler;
            if (dot_pos != handler.npos)
            {
                ns = handler.substr(0, dot_pos);
                handler = handler.substr(dot_pos + 1);
            }
            else
            {
                handler.clear();
            }

            auto mod_it = mods.find(ns);

            if (mod_it == mods.end())
            {
                req_log->info("No module {} was found for the last request", ns);
                std::string res = "no such module failed";
                socket_.async_send_to(boost::asio::buffer(res), ep, [res = std::move(res)](auto&, auto){});
                return;
            }

            auto reg = mod_it->second.reg;
            auto res = handler.empty() ? reg->post(body, ci) : reg->post(handler.c_str(), body, ci);

            if (!res)
            {
                req_log->info("No handler was found for the last request");
            }

            if (ci.res)
            {
                ci.log->info("Response: {}", ci.res.get());
                socket_.async_send_to(boost::asio::buffer(ci.res.get()), ep,
                        [](boost::system::error_code, std::size_t){});
            }

            req_log->info("Request handled successfully");
        }
        catch (std::exception& err)
        {
            req_log->error("Handling failed: {}", err.what());
            std::string res = "request failed";
            socket_.async_send_to(boost::asio::buffer(res), ep, [res = std::move(res)](auto&, auto){});
        }
    }

    void do_receive()
    {
        socket_.async_receive_from(boost::asio::buffer(data_, max_length), ep,
                [this](boost::system::error_code ec, std::size_t bytes_recvd)
                {
                    if (!ec && bytes_recvd > 0)
                    {
                        handle_packet(tos::span<const uint8_t>(data_, bytes_recvd), ep);
                    }

                    do_receive();
                });
    }

    std::unordered_map<std::string, module_info> mods;

    udp::socket socket_;
    enum { max_length = 1024 };
    udp::endpoint ep;
    uint8_t data_[max_length];
};

int main(int argc, char** argv)
{
    using namespace boost;

    asio::io_context io;

    server s(io);

    auto env = boost::this_process::environment();

    auto conf_path = argc > 1 ? argv[1] : env["ZAP_ENTRY"].to_string().c_str();
    std::ifstream conf_file{conf_path};

    auto conf = nlohmann::json::parse(conf_file);

    dll::shared_library lib(conf["module"].get<std::string>());

    s.load_module(conf["name"], std::move(lib));

    if (conf.find("auth") != conf.end())
    {
        auto addr = boost::asio::ip::make_address(conf["auth"]["host"].get<std::string>());
        auth = new rem_auth(io, udp::endpoint(addr, conf["auth"]["port"].get<uint16_t>()));
    }

    auto log = spdlog::stderr_color_mt("zap-system");

    log->info("Zap running in port {}", s.get_port());

    std::vector<std::thread> threads(std::thread::hardware_concurrency());

    for (auto& t : threads)
    {
        t = std::thread([&]{
            io.run();
        });
    }

    io.run();

    for (auto& t : threads)
    {
        t.join();
    }
}
