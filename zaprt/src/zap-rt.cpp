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

int main(int argc, char** argv)
{
    using namespace boost;
    using boost::asio::ip::udp;

    auto log = spdlog::stderr_color_mt("zap-system");

    asio::io_context io;

    zap::udp_server s(io);

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

    if (conf.find("modules") == conf.end())
    {
        log->error("No modules key in configuartion!");
        return 1;
    }

    auto modules = conf["modules"];

    for (nlohmann::json& mod : modules)
    {
        auto object_path = mod["module"].get<std::string>();
        auto ns = mod["name"].get<std::string>();
        log->info("Loading module from \"{}\" to namespace \"{}\"", object_path, ns);
        dll::shared_library lib(object_path);
        s.load_module(ns, std::move(lib));
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
