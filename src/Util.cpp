#include "Util.h"

namespace Util {
	std::vector<uint64_t> mysqlx_raw_as_u64_vector(const mysqlx::Value& in_value) {
		std::vector<uint64_t> out;

		const auto bytes = in_value.getRawBytes();
		auto ptr = reinterpret_cast<const std::byte*>(bytes.first);
		auto end = reinterpret_cast<const std::byte*>(bytes.first) + bytes.second;

		while (ptr != end) {
			static constexpr std::byte carry_flag{0b1000'0000};
			static constexpr std::byte value_mask{0b0111'1111};

			uint64_t v = 0;
			uint64_t shift = 0;
			bool is_carry;
			do {
				auto byte = *ptr;
				is_carry = (byte & carry_flag) == carry_flag;
				v |= std::to_integer<uint64_t>(byte & value_mask) << shift;

				++ptr;
				shift += 7;
			} while (is_carry && ptr != end && shift <= 63);

			out.push_back(v);
		}

		return out;
	}

	std::chrono::year_month_day read_date(const mysqlx::Value& value) {
		const auto vector = mysqlx_raw_as_u64_vector(value);
		if (vector.size() < 3)
			throw std::out_of_range{"Value is not a valid DATE"};

		return std::chrono::year{static_cast<int>(vector.at(0))} / static_cast<int>(vector.at(1)) / static_cast<int>(vector.at(2));
	}

	std::chrono::system_clock::time_point read_date_time(const mysqlx::Value& value) {
		const auto vector = mysqlx_raw_as_u64_vector(value);
		if (vector.size() < 3)
			throw std::out_of_range{"Value is not a valid DATETIME"};

		auto ymd = std::chrono::year{ static_cast<int>(vector.at(0)) } / static_cast<int>(vector.at(1)) / static_cast<int>(vector.at(2));
		auto sys_days = std::chrono::sys_days{ ymd };

		auto out = std::chrono::system_clock::time_point(sys_days);

		auto it = vector.begin() + 2;
		auto end = vector.end();

		if (++it == end)
			return out;
		out += std::chrono::hours{*it};

		if (++it == end)
			return out;
		out += std::chrono::minutes{*it};

		if (++it == end)
			return out;
		out += std::chrono::seconds{*it};

		if (++it == end)
			return out;
		out += std::chrono::microseconds{*it};

		return out;
	}
}