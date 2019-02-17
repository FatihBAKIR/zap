//
// Created by fatih on 2/16/19.
//

#pragma once
#include <flatbuffers/flatbuffers.h>
#include <bb/span.hpp>
#include <bb/function_traits.hpp>

namespace bb
{
    namespace detail
    {
        template <class T, class FunT, class... RestT>
        constexpr auto do_parse_flatbuf(FunT&& fn, type_list<T, RestT...>) noexcept
        {
            using Traits = function_traits<FunT>;
            using FBType = std::remove_const_t<std::remove_pointer_t<T>>;

            if constexpr (Traits::arity == 2)
            {
                return [=](tos::bytes s, client_id_t dev_id)
                {
                    fn(flatbuffers::GetRoot<FBType>(s.data()), dev_id);
                };
            }
            else if constexpr (Traits::arity == 1)
            {
                return [=](tos::bytes s, client_id_t)
                {
                    fn(flatbuffers::GetRoot<FBType>(s.data()));
                };
            }
        }
    }

    template <class FunT>
    constexpr auto flatbuf(FunT&& fn) noexcept
    {
        using Traits = function_traits<FunT>;
        return detail::do_parse_flatbuf(std::forward<FunT>(fn), typename Traits::arg_ts{});
    };
}