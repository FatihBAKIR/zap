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
        socket_.async_receive(
                boost::asio::buffer(data_, max_length),
                [this](boost::system::error_code ec, std::size_t bytes_recvd)
                {
                    if (!ec && bytes_recvd > 0)
                    {
                        auto req = flatbuffers::GetRoot<bb::cloud::Request>(data_);

                        auto body = tos::span<const uint8_t>(req->body()->data(), req->body()->size());

                        auto token = req->token()->str();

                        auto id = authenticate(token);

                        auto handler = req->handler()->c_str();

                        spdlog::get("req-log")->info("Got request on \"{}\" with body size {}", handler, body.size());

                        auto res = reg->post(handler, body, id);

                        if (!res)
                        {
                            spdlog::get("req-log")->info("No handler was found for the last request");
                        }
                    }
                    do_receive();
                });
    }

    boost::dll::shared_library lib;
    bb::registrar* reg;
private:
    udp::socket socket_;
    enum { max_length = 1024 };
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

    auto log = spdlog::stderr_color_mt("info");

    log->info("Zap running in port 9993");
    
    io.run();
}
