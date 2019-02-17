//
// Created by fatih on 2/16/19.
//

#include <jwt/jwt.hpp>
#include <bb/cloud.hpp>

bb::client_id_t authenticate(const std::string& token)
{
    using namespace jwt::params;

    auto dec_obj = jwt::decode(token, algorithms({"HS256"}), secret("sekkret"));

    nlohmann::json json = dec_obj.payload().create_json_obj();

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
