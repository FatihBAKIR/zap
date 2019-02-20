#pragma once

#include "handler.hpp"

namespace zap
{
	template <class... Lst>
	struct registry
	{
		std::tuple<Lst...> tup;

		constexpr registry(const Lst&... args) noexcept
			: tup(args...)
		{}

		template <class Name, class FuncT>
		constexpr auto attach(const handler<Name, FuncT>& i) const
		{
			using ret_t = registry<Lst..., handler<Name, FuncT>>;
			return ret_t{ std::tuple_cat(tup, std::tuple(i)) };
		}
	};

	registry() -> registry<>;

	template <class... Types>
	registry(const Types&...) -> registry<Types...>;

    namespace detail
    {
        template <class X, class Y>
        constexpr registry<handler<X, Y>> export_(const handler<X, Y>& single)
        {
            return registry(std::move(single));
        }

        template <class X, class Y>
        constexpr registry<handler<X, Y>> export_(handler<X, Y>&& single)
        {
            return registry(std::move(single));
        }

        template <class... Lst>
        constexpr registry<Lst...> export_(const registry<Lst...>& reg)
        {
            return reg;
        }

        template <class... Lst>
        constexpr registry<Lst...> export_(registry<Lst...>&& reg)
        {
            return reg;
        }

        template <class FunT>
        constexpr auto export_(FunT&& reg)
        {
            return export_(handler(std::forward<FunT>(reg)));
        }
    }
}
