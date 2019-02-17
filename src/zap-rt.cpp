#include <bb/zap.hpp>
#include <boost/dll.hpp>
#include <iostream>

#include <flatbuffers/flatbuffers.h>
#include <ip_addr_generated.h>
#include <vector>
#include <bb/cloud.hpp>

std::vector<uint8_t> get_ip_buf()
{
    flatbuffers::FlatBufferBuilder builder(1024);

    auto str = builder.CreateString("192.168.2.19");
    bb::cloud::IPAddrBuilder build(builder);
    build.add_addr(str);
    auto addr = build.Finish();
    builder.Finish(addr);

    return std::vector<uint8_t>(builder.GetBufferPointer(), builder.GetBufferPointer() + builder.GetSize());
}

void run(bb::registrar& h)
{
	auto buf = get_ip_buf();
    auto ip = flatbuffers::GetRoot<bb::cloud::IPAddr>(buf.data());

    bb::device_id_t id(1234);
    h.post("handle_ip", *ip, id);
}

bb::registrar bb_register(bb::registrar);

int main(int argc, char** argv)
{
    auto h = bb_register({});
    run(h);
}
