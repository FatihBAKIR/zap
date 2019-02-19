//
// Created by fatih on 2/19/19.
//

#include <bb/handler.hpp>
#include <bb/dynamic.hpp>
#include <jwt/jwt.hpp>

static nlohmann::json authenticate(tos::span<const uint8_t> token, zap::call_info& ci)
{
    using namespace jwt::params;

    jwt::string_view sv((const char*)token.data(), token.size());
    auto dec_obj = jwt::decode(sv, algorithms({"HS256"}), secret("sekkret"));

    nlohmann::json json = dec_obj.payload().create_json_obj();

    ci.log->info("auth succeeded!");

    return dec_obj.payload().create_json_obj();
    //return bb::deserialize(json);
}

ZAP(authenticate);