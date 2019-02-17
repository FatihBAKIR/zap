#pragma once

#include <cstdint>

namespace bb
{
	struct org_id_t
	{
		uint32_t org_id;
	};

	class device_id_t
	{
		struct org_dev_id_t
		{
			org_id_t org_id;
			uint32_t dev_id;
		};
	public:
		constexpr device_id_t(org_id_t org, uint32_t dev) :
			org_dev_id(org_dev_id_t{ org, dev })
		{
		}

		constexpr device_id_t(uint64_t uniq) : unique_id(uniq) 
		{
		}

		constexpr uint64_t get_unique_id() const
		{
			return unique_id;
		}

		constexpr org_dev_id_t get_org_dev_id() const
		{
			return org_dev_id;
		}

	private:
		union {
			org_dev_id_t org_dev_id;
			uint64_t unique_id;
		};
	};
}