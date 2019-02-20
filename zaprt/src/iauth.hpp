//
// Created by fatih on 2/19/19.
//

#pragma once

#include <bb/auth.hpp>
#include <string>
#include <memory>

namespace zap
{
struct iauth
{
    virtual bb::client_id_t authenticate(const std::string&) = 0;
    virtual ~iauth() = default;
};

    std::unique_ptr<iauth> make_null_auth();
}
