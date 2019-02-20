//
// Created by fatih on 2/19/19.
//

#include "iauth.hpp"

namespace {
struct null_authenticator : zap::iauth
{
    bb::client_id_t authenticate(const std::string &string) override {
        return {};
    }
};
}

namespace zap
{
    std::unique_ptr<iauth> make_null_auth()
    {
        return std::make_unique<null_authenticator>();
    }
}