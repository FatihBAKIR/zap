#include <iostream>

#include <bb/dynamic.hpp>
#include <fbs/ip_addr_generated.h>
#include <bb/flatbuf.hpp>
#include <nlohmann/json.hpp>

static auto handle_ip(const zap::cloud::IPAddr* ip, zap::call_info& ci)
{
	ci.log->info("{} : {}", ip->gw_id(), ip->addr()->str());

	nlohmann::json j;
	j["status"] = "success";
	j["new_ip"] = ip->addr()->str();

	return j;
}

constexpr auto x = zap::handler("foo", zap::flatbuf(&handle_ip));

ZAP(zap::registry(x));