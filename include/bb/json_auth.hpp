//
// Created by fatih on 2/19/19.
//

#pragma once

#include "auth.hpp"
#include <nlohmann/json_fwd.hpp>

namespace bb
{
    client_id_t deserialize(const nlohmann::json& j);
}