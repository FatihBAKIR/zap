#pragma once

#include <map>
#include <functional>
#include "span.hpp"
#include <bb/function_traits.hpp>
namespace bb
{
	namespace detail
	{
		struct event_base {};

		template <int N>
		struct event : event_base {
			constexpr static inline auto event_id = N;
		};
		
		template <size_t N>
		constexpr unsigned int hash(const char(&x)[N])
		{
			unsigned int h = 0;
			for (int i = 0; i < N; ++i)
			{
				h += x[i];
			}
			return h;
		}

		template <class T>
		constexpr unsigned int do_type_id()
		{
			return typeid(T).hash_code();
		}

		template <class T>
		constexpr unsigned int type_id()
		{
			return do_type_id<std::remove_const_t<std::remove_reference_t<T>>>();
		}
	}	
	
	namespace events
	{
		template <class ArgTs>
		struct put_t : detail::event<1> {
		};

		template <class ArgTs>
		constexpr inline put_t<ArgTs> put{};
	}

	template <class T>
    struct deser_traits
    {
	    using type = T;
    };

	template <class T>
    typename deser_traits<T>::type
    deser(tos::span<const uint8_t>);

	template <class T>
    typename deser_traits<T>::type
    deser(typename deser_traits<T>::type x)
    {
        return x;
    }

	template <class FunT>
	struct handler
	{
		const char* name;
		FunT fun;
	};

	template <class FunT>
	handler(const char*, FunT) -> handler<FunT>;

	class registrar
	{
		template <class... ArgTs, class FuncT>
		registrar& do_attach(const char* name, type_list<ArgTs...>, FuncT&& f)
		{
			using FunT = std::remove_reference_t<std::decay_t<FuncT>>;

			fun fn;
			fn.data = new FunT(std::forward<FuncT>(f));
			fn.fp = [](void* args, void* d)
			{
				auto& h = *static_cast<FunT*>(d);
				auto& a = *static_cast<std::tuple<ArgTs&...>*>(args);

				std::apply(h, a);
			};

			auto key = std::tuple<std::string, size_t, size_t>{ name, sizeof...(ArgTs), (0 + ... + detail::type_id<ArgTs>()) };
			m_handlers.emplace(key, fn);

			return *this;
		}

	public:
		template <class FuncT>
		registrar& attach(const char* name, FuncT&& f)
		{
			using ArgTs = typename bb::function_traits<std::remove_reference_t <FuncT>>::arg_ts;
			return do_attach(name, ArgTs{}, std::forward<FuncT>(f));
		}

		template <class FuncT>
		registrar& attach(const handler<FuncT>& i)
		{
			return attach(i.name, i.fun);
		}

		template <class... ArgTs>
		bool post(const char* name, ArgTs&&... args)
		{
			auto key = std::tuple<std::string, size_t, size_t>{
			    name, sizeof...(ArgTs), (0 + ... + detail::type_id<ArgTs>())
			};

			auto it = m_handlers.find(key);
			if (it == m_handlers.end()) return false;

			std::tuple<ArgTs&...> a(args...);
			it->second.fp(&a, it->second.data);
			return true;
		}

	private:
		struct fun
		{
			void* data;
			void(*fp)(void*, void*);
		};

		std::map<std::tuple<std::string, size_t, size_t>, fun> m_handlers;
	};
}

#define ZAP_EXPORT __attribute__((visibility("default")))