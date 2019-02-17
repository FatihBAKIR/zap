#include <bb/zap.hpp>
#include <boost/dll.hpp>
#include <iostream>

#include <flatbuffers/flatbuffers.h>
#include <vector>
#include <bb/cloud.hpp>

#include <boost/asio.hpp>

#include <req_generated.h>

#include <boost/process.hpp>

#include <spdlog/spdlog.h>
#include "spdlog/sinks/stdout_color_sinks.h"

#include <thread>
#include <bb/handler.hpp>

using boost::asio::ip::udp;

bb::client_id_t authenticate(const std::string& token);

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
                        auto req = flatbuffers::GetRoot<bb::cloud::Request>(data_);
                        auto ver = flatbuffers::Verifier(data_, bytes_recvd);
                        bool ok = bb::cloud::VerifyRequestBuffer(ver);

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

                        auto handler = req->handler()->c_str();

                        req_log->info(
                                "Got request on \"{}\" with body size {} from {}:{}",
                                handler, body.size(), remote_addr, remote_port);

                        try
                        {
                            auto id = authenticate(token);

                            auto fun_log = spdlog::get(handler);

                            bb::call_info ci;
                            ci.client = id;
                            ci.log = fun_log ? fun_log : spdlog::stdout_color_mt(handler);

                            auto res = reg->post(handler, body, ci);

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

    boost::dll::shared_library lib;
    bb::registrar* reg;
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

    s.lib = std::move(lib);
    s.reg = &s.lib.get<bb::registrar>("registry");

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
