#include <bb/zap.hpp>
#include <boost/dll.hpp>
#include <iostream>

#include <flatbuffers/flatbuffers.h>
#include <vector>
#include <bb/cloud.hpp>

#include <boost/asio.hpp>

#include <req_generated.h>

using boost::asio::ip::udp;

class server
{
public:
    server(boost::asio::io_context& io_context, short port)
            : socket_(io_context, udp::endpoint(udp::v4(), port))
    {
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

                        bb::client_id_t id = bb::user_id_t{1234};
                        reg->post(req->handler()->c_str(), body, id);
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

    dll::shared_library lib(argv[1]);

    s.lib = std::move(lib);
    s.reg = &s.lib.get<bb::registrar>("registry");

    io.run();
}
