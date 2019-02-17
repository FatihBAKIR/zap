#pragma once

#include <map>
#include <functional>

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

	class registrar
	{
	public:
		template <class... ArgTs, class FuncT>
		registrar& attach(const char* name, FuncT&& f)
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

		std::map<std::string, std::map<std::tuple<size_t, size_t>, fun>> m_woah;

		std::map<std::tuple<std::string, size_t, size_t>, fun> m_handlers;
	};
}

#define ZAP_EXPORT __attribute__((visibility("default")))