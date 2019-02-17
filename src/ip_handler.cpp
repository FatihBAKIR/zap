#include <iostream>
#include <string>

#include <bb/zap.hpp>
#include <bb/cloud.hpp>
#include <flatbuffers/flatbuffers.h>
#include <flatbuffers/minireflect.h>
#include <ip_addr_generated.h>

using namespace bb::cloud;

static void handle_ip(const IPAddr& ip, const bb::device_id_t& from)
{
	std::cout << from.get_unique_id() << " : " << ip.addr()->str() << '\n';
}

ZAP_EXPORT bb::registrar
bb_register(bb::registrar r)
{
    using namespace bb::events;
	return r
		.attach<IPAddr, bb::device_id_t>("handle_ip", handle_ip);
}
