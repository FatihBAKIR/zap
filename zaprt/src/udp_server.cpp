//
// Created by fatih on 2/19/19.
//

#include <boost/asio/ip/udp.hpp>
#include "udp_server.hpp"
#include "module_info.hpp"
#include <flatbuffers/flatbuffers.h>
#include <fbs/req_generated.h>
#include <spdlog/spdlog.h>
#include "spdlog/sinks/stdout_color_sinks.h"

using boost::asio::ip::udp;

namespace zap
{
    struct udp_server::impl
    {
        std::unordered_map<std::string, zap::module_info> mods;

        std::unique_ptr<iauth> auth;
        udp::socket socket_;
        enum { max_length = 1024 };
        udp::endpoint ep;
        uint8_t data_[max_length];

        void handle_packet(tos::span<const uint8_t> packet, const udp::endpoint& from);

        void handle_receive(boost::system::error_code ec, std::size_t bytes_recvd);
        void do_receive();

        impl(boost::asio::io_context& io) :
            socket_(io, udp::endpoint(udp::v4(), 0))
        {
            auto req_log = spdlog::stderr_color_mt("req-log");
            do_receive();
        }
    };

    void udp_server::impl::handle_packet
        (tos::span<const uint8_t> packet, const udp::endpoint& from)
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

    void udp_server::impl::do_receive() {
        socket_.async_receive_from(
                boost::asio::buffer(data_, max_length),
                ep, [this](auto& a, auto& b){handle_receive(a, b);});
    }

    void udp_server::impl::handle_receive(boost::system::error_code ec, std::size_t bytes_recvd) {
        if (!ec && bytes_recvd > 0)
        {
            handle_packet(tos::span<const uint8_t>(data_, bytes_recvd), ep);
        }

        do_receive();
    }

    void udp_server::set_auth(std::unique_ptr<iauth> auth) {
        m_impl->auth = std::move(auth);
    }

    void udp_server::load_module(std::string_view ns, boost::dll::shared_library &&lib) {
        auto [it, ins] = m_impl->mods.emplace(ns, std::move(lib));
        auto log = spdlog::get("zap-system");
        if (ins)
        {
            log->info("Namespace \"{}\" loaded", ns);
            for (auto& handler : it->second.reg->get_handlers())
            {
                if (handler != "moddef")
                {
                    log->info("Handler added: \"{}.{}\"", std::string(ns), handler);
                } else {
                    log->info("Handler added: \"{}\"", std::string(ns));
                }
            }
        }
        else
        {
            log->warn("Namespace {} failed to load!", ns);
        }
    }

    udp_server::udp_server(boost::asio::io_context &io_context)
        : m_impl(std::make_unique<impl>(io_context))
    {
        m_impl->auth = make_null_auth();
    }

    uint16_t udp_server::get_port() const
    {
        return m_impl->socket_.local_endpoint().port();
    }

    udp_server::~udp_server() = default;
}