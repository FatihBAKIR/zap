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
		template <template<class...> class EventT, class... ArgTs, class FuncT>
		registrar& attach(const EventT<ArgTs...>& e, FuncT&& f)
		{
			static_assert(std::is_base_of_v<detail::event_base, EventT<ArgTs...>>, "events must inherit from event_base!");

			using FunT = std::remove_reference_t<std::decay_t<FuncT>>;

			fun fn;
			fn.data = new FunT(std::forward<FuncT>(f));
			fn.fp = [](void* args, void* d)
			{
				auto& h = *static_cast<FunT*>(d);
				auto& a = *static_cast<std::tuple<ArgTs&...>*>(args);

				std::apply(h, a);
			};

			auto key = std::tuple<int, size_t, size_t>{ e.event_id, sizeof...(ArgTs), (0 + ... + detail::type_id<ArgTs>()) };
			m_handlers.emplace(key, fn);

			return *this;
		}

		template <template<class...> class EventT, class... ArgTs>
		bool post(const EventT<ArgTs...>& e, ArgTs&&... args)
		{
			static_assert(std::is_base_of_v<detail::event_base, EventT<ArgTs...>>, "events must inherit from event_base!");

			auto key = std::tuple<int, size_t, size_t>{ e.event_id, sizeof...(ArgTs), (0 + ... + detail::type_id<ArgTs>()) };

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
		
		std::map<std::tuple<int, size_t, size_t>, fun> m_handlers;
	};
}