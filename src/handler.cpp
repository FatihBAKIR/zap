//
// Created by fatih on 2/17/19.
//

#include <bb/handler.hpp>
#include <nlohmann/json.hpp>
#include <sstream>

namespace bb
{
    ZAP_EXPORT void response::set(const nlohmann::json &j) {
        std::ostringstream oss;
        oss << j;
        m_res = oss.str();
    }
}