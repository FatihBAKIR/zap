//
// Created by fatih on 2/17/19.
//

#pragma once

#include "auth.hpp"
#include <spdlog/spdlog.h>
#include <nlohmann/json_fwd.hpp>
#include <any>

#define ZAP_PUBLIC __attribute__((visibility("default")))

namespace zap
{
    struct response
    {
        void set(const char* r)
        {
            m_res = r;
        }

        void set(std::string r)
        {
            m_res = r;
        }

        void set(const nlohmann::json& j) ZAP_PUBLIC;

        explicit operator bool() const {
            return bool(m_res);
        }

        const std::string& get() const {
            return *m_res;
        }
    private:
        std::optional<std::string> m_res;
    };

    struct call_info
    {
        bb::client_id_t client;
        std::shared_ptr<spdlog::logger> log;
        response res;

        call_info() = default;
        call_info(const call_info&) = delete;
    };

    struct default_event
    {
        const char* name = "moddef";
    };

    struct dynamic_named_event
    {
        const char* name;
    };

    template <class EventTag, class FunT>
    struct handler : EventTag
    {
        FunT fun;

        template <class FT>
        constexpr handler(FT&& f) noexcept
                : EventTag{}, fun{std::forward<FT>(f)} {}

        template <class ETagT, class FT>
        constexpr handler(ETagT&& e, FT&& f) noexcept
                : EventTag{std::forward<ETagT>(e)}, fun{std::forward<FT>(f)} {}
    };

    template <class FunT>
    handler(FunT&& f) -> handler<default_event, FunT>;

    template <class FunT>
    handler(const char*, FunT&& f) -> handler<dynamic_named_event, FunT>;

    template <class EventTag, class FunT>
    handler(EventTag&& e, FunT&& f) -> handler<EventTag, FunT>;

    template <class Name, class FunT>
    handler<Name, FunT> make_handler(FunT&& f)
    {
        return { std::forward(f) };
    }
}
