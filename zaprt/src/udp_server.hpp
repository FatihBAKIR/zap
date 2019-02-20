//
// Created by fatih on 2/19/19.
//

#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/dll/shared_library.hpp>
#include "iauth.hpp"

namespace zap
{
    class udp_server
    {
    public:
        explicit udp_server(boost::asio::io_context& io_context);
        explicit udp_server(boost::asio::io_context& io_context, uint16_t port);

        void set_auth(std::unique_ptr<iauth> auth);

        void load_module(std::string_view ns, boost::dll::shared_library&& lib);
        uint16_t get_port() const;

        ~udp_server();
    private:
        struct impl;
        std::unique_ptr<impl> m_impl;
    };
}
