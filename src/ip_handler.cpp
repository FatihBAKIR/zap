#include <iostream>

#include <bb/zap.hpp>
#include <bb/cloud.hpp>
#include <ip_addr_generated.h>
#include <bb/flatbuf.hpp>

static void handle_ip(const bb::cloud::IPAddr* ip, bb::call_info ci)
{
	ci.log->info("{} : {}", ip->gw_id(), ip->addr()->str());
}

auto x = bb::handler{ "handle_ip", bb::flatbuf(&handle_ip) };

extern "C" ZAP_EXPORT bb::registrar registry = bb::registrar().attach(x);