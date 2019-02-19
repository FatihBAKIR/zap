//
// Created by fatih on 2/16/19.
//

#include <nlohmann/json.hpp>
#include <bb/auth.hpp>
#include <zap/json_auth.hpp>

namespace bb
{
    client_id_t deserialize(const nlohmann::json& json)
    {
        switch (json["type"].get<int>())
        {
            case 0:
                //user
                return bb::user_id_t{ json["obj"].get<uint32_t>() };
            case 1:
                // gateway
                bb::org_id_t org{ json["obj"]["owner"].get<uint32_t>() };
                return bb::device_id_t { org, json["obj"]["gw"].get<uint32_t>() };
        }
    }
}