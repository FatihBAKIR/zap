#include <iostream>
#include <string>

#include <bb/zap.hpp>

struct ip_addr
{
	uint8_t addr[4];
};

void handle_ip(const ip_addr& ip, int x = 5)
{
	std::cout << int(ip.addr[0]) << '\n';
}

auto bb_register(bb::registrar& r)
{
	using namespace bb::events;
	return r
		.attach(put<ip_addr>, handle_ip);
}

int main()
{
	auto h = bb_register(bb::registrar{});

	auto handled = h.post(bb::events::put<int>, 5);
	std::cout << "post(put<int>, 5) handled: " << handled << '\n';

	handled = h.post(bb::events::put<float>, 5.f);
	std::cout << "post(put<float>, 5.f) handled: " << handled << '\n';

	std::cin.get();
	return 0;
}
