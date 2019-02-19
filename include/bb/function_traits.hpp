//
// Created by fatih on 2/16/19.
//

#pragma once

#include <type_traits>

namespace zap
{
    template <class...> struct type_list {};

    namespace detail
    {
        template <class T>
        using clean_t = std::remove_const_t<std::remove_reference_t<T>>;

        template <class FunT>
        struct function_traits;

        template <class RetT, class... ArgTs>
        struct function_traits_base
        {
            using ret_t = RetT;
            using arg_ts = type_list<clean_t<ArgTs>...>;
            static constexpr auto arity = sizeof...(ArgTs);
        };

        template <class RetT, class... ArgTs>
        struct function_traits<RetT(*)(ArgTs...)> : function_traits_base<RetT, ArgTs...> {};

        template <class RetT, class... ArgTs>
        struct function_traits<RetT(&)(ArgTs...)> : function_traits_base<RetT, ArgTs...> {};

        template <class ClassT, class RetT, class... ArgTs>
        struct function_traits<RetT (ClassT::*) (ArgTs...)> : function_traits_base<RetT, ArgTs...> {};

        template <class ClassT, class RetT, class... ArgTs>
        struct function_traits<RetT (ClassT::*) (ArgTs...) const> : function_traits_base<RetT, ArgTs...> {};

        template <class ClassT, class RetT, class... ArgTs>
        struct function_traits<RetT (ClassT::*) (ArgTs...) noexcept> : function_traits_base<RetT, ArgTs...> {};

        template <class ClassT, class RetT, class... ArgTs>
        struct function_traits<RetT (ClassT::*) (ArgTs...) const noexcept> : function_traits_base<RetT, ArgTs...> {};
    }

    template <class T>
    struct function_traits
            : detail::function_traits<decltype(&T::operator())> {};

    template <class RetT, class...ArgTs>
    struct function_traits<RetT(&)(ArgTs...)>
            : detail::function_traits<RetT(&)(ArgTs...)> {};

    template <class RetT, class...ArgTs>
    struct function_traits<RetT(*)(ArgTs...)>
            : detail::function_traits<RetT(*)(ArgTs...)> {};
}