//
// Created by fatih on 2/18/19.
//

#pragma once
#include <unordered_map>
#include <functional>
#include "span.hpp"
#include "handler.hpp"
#include "registry.hpp"
#include <bb/function_traits.hpp>

namespace zap
{
    class dynamic_registry
    {
    public:
        template <class ArgT>
        bool post(const char* name, ArgT&& arg, call_info& ci)
        {
            auto it = m_funs.find(std::string(name));
            if (it == m_funs.end()) return false;

            std::tuple<ArgT&, call_info&> a(arg, ci);
            it->second.fp(&a, it->second.data);
            return true;
        }

        template <class ArgT>
        bool post(ArgT&& arg, call_info& ci)
        {
            auto it = m_funs.find(std::string("moddef"));
            if (it == m_funs.end()) return false;

            std::tuple<ArgT&, call_info&> a(arg, ci);
            it->second.fp(&a, it->second.data);
            return true;
        }

        template <class... Lst>
        dynamic_registry(const registry<Lst...>& x)
        {
            do_attach(x.tup, std::make_index_sequence<sizeof...(Lst)>{});
        }

    private:

        template <class... Lst, size_t... Is>
        void do_attach(const std::tuple<Lst...>& x, std::index_sequence<Is...>)
        {
            (attach(std::get<Is>(x)), ...);
        }

        template <class ArgT, class FuncT>
        void do_attach(const char* name, type_list<ArgT, call_info> arg, FuncT&& f)
        {
            using FunT = std::remove_reference_t<std::decay_t<FuncT>>;

            fun fn;
            fn.data = new FunT(std::forward<FuncT>(f));
            fn.fp = [](void* args, void* d)
            {
                auto& h = *static_cast<FunT*>(d);
                auto& a = *static_cast<std::tuple<ArgT&, call_info&>*>(args);

                std::apply(h, a);
            };

            m_funs.emplace(std::string(name), fn);
        }

        template <class FuncT>
        void attach(const char* name, FuncT&& f)
        {
            using ArgTs = typename zap::function_traits<std::remove_reference_t <FuncT>>::arg_ts;
            do_attach(name, ArgTs{}, std::forward<FuncT>(f));
        }

        template <class Name, class FuncT>
        void attach(const handler<Name, FuncT>& i)
        {
            attach(i.name, i.fun);
        }

        struct fun
        {
            void* data;
            void(*fp)(void*, void*);
        };

        std::unordered_map<std::string, fun> m_funs;
    };
}

#define ZAP_DYN(x) \
    extern "C" { ZAP_PUBLIC zap::dynamic_registry registry(x); }

#define ZAP(x) \
    constexpr static auto ___y = zap::detail::export_(x); \
    ZAP_DYN(___y)
