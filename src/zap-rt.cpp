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

using boost::asio::ip::udp;

zap::client_id_t authenticate(const std::string& token);

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

class server
{
public:
    server(boost::asio::io_context& io_context, short port)
            : socket_(io_context, udp::endpoint(udp::v4(), port))
    {
        auto req_log = spdlog::stderr_color_mt("req-log");
        do_receive();
    }

    void do_receive()
    {
        socket_.async_receive_from(
                boost::asio::buffer(data_, max_length), ep,
                [this](boost::system::error_code ec, std::size_t bytes_recvd)
                {
                    if (!ec && bytes_recvd > 0)
                    {
                        auto req = flatbuffers::GetRoot<zap::cloud::Request>(data_);
                        auto ver = flatbuffers::Verifier(data_, bytes_recvd);
                        bool ok = zap::cloud::VerifyRequestBuffer(ver);

                        auto remote_addr = ep.address().to_string();
                        auto remote_port = ep.port();

                        auto req_log = spdlog::get("req-log");
                        if (!ok)
                        {
                            req_log->error("Received garbage from {}:{}", remote_addr, remote_port);
                            goto end;
                        }

                        auto body = tos::span<const uint8_t>(req->body()->data(), req->body()->size());

                        auto token = req->token()->str();

                        auto handler = req->handler()->str();

                        req_log->info(
                                "Got request on \"{}\" with body size {} from {}:{}",
                                handler, body.size(), remote_addr, remote_port);

                        try
                        {
                            auto id = authenticate(token);

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

                            auto reg = mods.find(ns)->second.reg;
                            auto res = handler.empty() ? reg->post(body, ci) : reg->post(handler.c_str(), body, ci);

                            if (!res)
                            {
                                req_log->info("No handler was found for the last request");
                            }

                            if (ci.res)
                            {
                                ci.log->info("Response: {}", ci.res.get());
                                socket_.async_send_to(boost::asio::buffer(ci.res.get()), ep, [](boost::system::error_code, std::size_t){

                                });
                            }

                            req_log->info("Request handled successfully");
                        }
                        catch (std::exception& err)
                        {
                            req_log->error("Handling failed: {}", err.what());
                        }
                    }

                    end:
                    do_receive();
                });
    }

    std::unordered_map<std::string, module_info> mods;

private:
    udp::socket socket_;
    enum { max_length = 1024 };
    udp::endpoint ep;
    uint8_t data_[max_length];
};

int main(int argc, char** argv)
{
    using namespace boost;

    asio::io_context io;

    server s(io, 9993);

    auto env = boost::this_process::environment();

    dll::shared_library lib(argc > 1 ? argv[1] : env["ZAP_ENTRY"].to_string().c_str());

    s.mods.emplace("handle_ip", std::move(lib));

    auto log = spdlog::stderr_color_mt("zap-system");

    log->info("Zap running in port 9993");

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
