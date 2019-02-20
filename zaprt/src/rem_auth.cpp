//
// Created by fatih on 2/19/19.
//

#include <flatbuffers/flatbuffers.h>
#include <fbs/req_generated.h>
#include <zap/json_auth.hpp>
#include "rem_auth.hpp"
#include <nlohmann/json.hpp>

namespace
{
using boost::asio::ip::udp;
struct rem_auth : zap::iauth
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
}

namespace zap
{
    std::unique_ptr<iauth> make_remote_auth(boost::asio::io_context& ioc, const boost::asio::ip::udp::endpoint& e)
    {
        return std::make_unique<rem_auth>(ioc, e);
    }
}
