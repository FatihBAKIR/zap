//
// Created by fatih on 2/19/19.
//

#pragma once

#include <nlohmann/json.hpp>
#include "span.hpp"
#include "handler.hpp"
#include <string_view>

namespace zap
{
    template <class FunT>
    constexpr auto json(FunT&& fn) noexcept
    {
        return [=](tos::bytes b, zap::call_info& ci)
        {
            auto j = nlohmann::json::parse(std::string_view((const char*)b.begin(), b.size()));
            return fn(j, ci);
        };
    };
}