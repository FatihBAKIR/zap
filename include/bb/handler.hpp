//
// Created by fatih on 2/17/19.
//

#pragma once

#include "cloud.hpp"
#include <spdlog/spdlog.h>

namespace bb
{
    struct registrar;
    struct call_info
    {
        client_id_t client;
        std::shared_ptr<spdlog::logger> log;
    };
}