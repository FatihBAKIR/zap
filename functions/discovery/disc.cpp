//
// Created by fatih on 2/19/19.
//

#include <nlohmann/json.hpp>
#include <bb/handler.hpp>
#include <bb/span.hpp>
#include <bb/json.hpp>
#include <bb/dynamic.hpp>

struct end_point
{
    std::string host;
    uint16_t port;
};

struct reg_info
{
    std::string mod_name;
    end_point ep;
};

struct registered_info
{
    end_point ep;
};

std::map<std::string, std::vector<registered_info>> x;

void reg(const reg_info& info, zap::call_info& ci)
{
    ci.log->info("Registering module {} at {}:{}", info.mod_name, info.ep.host, info.ep.port);

    if (x.find(info.mod_name) == x.end())
    {
        x[info.mod_name].push_back(registered_info{ info.ep });
    }
}

void find(const std::string& name, zap::call_info& ci)
{
    if (x.find(name) == x.end())
    {
        ci.res.set(nlohmann::json(fmt::format("{} not found", name)));
        return;
    }

    auto& ep = x[name].back().ep;
    ci.res.set(nlohmann::json(fmt::format("{}:{}", ep.host, ep.port)));
}

void deser_json(const nlohmann::json& info, zap::call_info& ci)
{
    end_point ep;
    ep.port = info["port"];
    ep.host = info["host"];
    reg_info inf;
    inf.ep = ep;
    inf.mod_name = info["mod"];
    reg(inf, ci);
}

template <class FunT>
constexpr auto string(FunT&& fn) noexcept
{
    return [=](tos::span<const uint8_t> x, zap::call_info& ci)
    {
        return fn(std::string((const char*)x.data(), x.size()), ci);
    };
}

constexpr auto r = zap::handler("reg", zap::json(&deser_json));
constexpr auto f = zap::handler("find", string(&find));
constexpr auto funs = zap::registry(r, f);

ZAP(funs);