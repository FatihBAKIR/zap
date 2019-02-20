//
// Created by fatih on 2/19/19.
//


#include "iauth.hpp"
#include <memory>
#include <boost/asio/ip/udp.hpp>

namespace boost::asio {
    class io_context;
}

namespace zap
{
    std::unique_ptr<iauth> make_remote_auth(boost::asio::io_context& ioc, const boost::asio::ip::udp::endpoint& e);
}
