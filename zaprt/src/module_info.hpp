//
// Created by fatih on 2/19/19.
//

#pragma once

#include <boost/dll.hpp>
#include <bb/dynamic.hpp>

namespace zap
{
struct module_info
{
    explicit module_info(boost::dll::shared_library&& l) : lib(std::move(l))
    {
        reg = &lib.get<zap::dynamic_registry>("registry");
    }

    zap::dynamic_registry* reg;
private:
    boost::dll::shared_library lib;
};
}
