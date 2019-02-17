//
// Created by fatih on 2/17/19.
//

#pragma once

#include "cloud.hpp"
#include <spdlog/spdlog.h>
#include <nlohmann/json_fwd.hpp>
#include <any>

#define ZAP_EXPORT __attribute__((visibility("default")))

namespace bb
{
    struct response
    {
        void set(std::string r)
        {
            m_res = r;
        }

        void set(const nlohmann::json& j) ZAP_EXPORT;

        explicit operator bool() const {
            return bool(m_res);
        }

        const std::string& get() const {
            return *m_res;
        }
    private:
        std::optional<std::string> m_res;
    };

    struct registrar;
    struct call_info
    {
        client_id_t client;
        std::shared_ptr<spdlog::logger> log;
        response res;

        call_info() = default;
        call_info(const call_info&) = delete;
    };
}
