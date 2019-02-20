#include <iostream>
#include <vector>
#include <thread>

#include <boost/asio.hpp>
#include <boost/process.hpp>

#include <spdlog/spdlog.h>
#include "spdlog/sinks/stdout_color_sinks.h"

#include <nlohmann/json.hpp>

#include <bb/dynamic.hpp>
#include "iauth.hpp"
#include "module_info.hpp"
#include "rem_auth.hpp"
#include "udp_server.hpp"

struct disco_registrar
{
public:
    disco_registrar(
            boost::asio::io_context& ioc,
            const boost::asio::ip::udp::endpoint& disco_ep,
            const boost::asio::ip::udp::endpoint& local_ep)
            : sock(ioc), m_disco_ep(disco_ep), m_local_ep(local_ep), timer(ioc)
    {
        sock.open(boost::asio::ip::udp::v4());
        do_wait();
        log = spdlog::stderr_color_mt("discovery");
    }

    void add_ns(std::string_view ns)
    {
        namespaces.emplace_back(ns);
        handle_tick();
    }

private:

    void do_wait()
    {
        timer.expires_from_now(boost::posix_time::seconds(30));
        timer.async_wait([this](boost::system::error_code ec){
            if (ec) return;
            handle_tick();
            do_wait();
        });
    }

    void handle_tick()
    {
        nlohmann::json j;
        j["host"] = m_local_ep.address().to_string();
        j["port"] = m_local_ep.port();
        j["mods"] = namespaces;
        auto str = j.dump(0);
        auto ptr = &str[0];
        auto len = str.size();

        // TODO: create a proper flatbuffer request here!
        sock.async_send_to(boost::asio::buffer(ptr, len), m_disco_ep, [str = std::move(str), this](auto& ec, auto sent){
            if (ec)
            {
                log->error("Discovery registration failed! {}", ec.message());
            }
        });
    }

    std::shared_ptr<spdlog::logger> log;
    boost::asio::deadline_timer timer;
    std::vector<std::string> namespaces;
    boost::asio::ip::udp::endpoint m_disco_ep;
    boost::asio::ip::udp::endpoint m_local_ep;
    boost::asio::ip::udp::socket sock;
};

int main(int argc, char** argv)
{
    using namespace boost;
    using boost::asio::ip::udp;

    auto log = spdlog::stderr_color_mt("zap-system");

    asio::io_context io;

    auto env = this_process::environment();

    if (argc == 1 && env["ZAP_ENTRY"].empty())
    {
        log->error("No configuartion provided!");
        return 1;
    }

    auto conf_path = argc > 1 ? argv[1] : env["ZAP_ENTRY"].to_string().c_str();
    std::ifstream conf_file{conf_path};

    log->info("Zap Serverless Dynamic Runtime {}", "0.1.1");
    log->info("Loading Configuration From \"{}\"", conf_path);

    auto conf = nlohmann::json::parse(conf_file);

    uint16_t port = 0;
    if (conf.find("prefer-port") != conf.end())
    {
        port = conf["prefer-port"];
        log->info("Configuration prefers port {}", port);
    }

    if (conf.find("modules") == conf.end())
    {
        log->error("No modules key in configuartion!");
        return 1;
    }

    zap::udp_server s(io, port);
    std::unique_ptr<disco_registrar> registrar;

    if (conf.find("discovery") != conf.end())
    {
        auto addr = boost::asio::ip::make_address(conf["discovery"]["host"].get<std::string>());
        auto port = conf["discovery"]["port"].get<uint16_t>();
        log->info("Config has discovery server on {}:{}", addr.to_string(), port);
        auto local_addr = boost::asio::ip::make_address("127.0.0.1");
        registrar = std::make_unique<disco_registrar>(
                io,
                udp::endpoint(addr, port),
                udp::endpoint(local_addr, s.get_port()));
    }

    auto modules = conf["modules"];

    for (nlohmann::json& mod : modules)
    {
        auto object_path = mod["module"].get<std::string>();
        auto ns = mod["name"].get<std::string>();
        log->info("Loading module from \"{}\" to namespace \"{}\"", object_path, ns);
        dll::shared_library lib(object_path);
        s.load_module(ns, std::move(lib));
        if (registrar)
        {
            registrar->add_ns(ns);
        }
    }

    if (conf.find("auth") != conf.end())
    {
        auto addr = boost::asio::ip::make_address(conf["auth"]["host"].get<std::string>());
        auto port = conf["auth"]["port"].get<uint16_t>();
        log->info("Configuration has authentication on {}:{}", addr.to_string(), port);
        s.set_auth(zap::make_remote_auth(io, udp::endpoint(addr, port)));
    }

    std::vector<std::thread> threads(std::thread::hardware_concurrency());

    for (auto& t : threads)
    {
        t = std::thread([&]{
            io.run();
        });
    }

    log->info("Zap running in port {} with {} threads", s.get_port(), threads.size());

    for (auto& t : threads)
    {
        t.join();
    }
}
