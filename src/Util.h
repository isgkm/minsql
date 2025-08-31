#pragma once

#include <mysqlx/xdevapi.h>
#include <cstddef>
#include <vector>
#include <chrono>
#include <cstdint>
#include <array>
#include <vector>

#define MESSAGE_BOX_INFORMATION(str) wxMessageBox(str, "Information", wxICON_INFORMATION);
#define MESSAGE_BOX_WARNING(str) wxMessageBox(str, "Warning", wxICON_WARNING);
#define MESSAGE_BOX_ERROR(str) wxMessageBox(str, "Error", wxICON_ERROR);
#define WXSTR_TO_STD(str) static_cast<std::string>(str)

namespace Util {
	std::vector<uint64_t>
	mysqlx_raw_as_u64_vector(const mysqlx::Value& in_value);

	std::chrono::year_month_day
	read_date(const mysqlx::Value& value);

	std::chrono::system_clock::time_point
	read_date_time(const mysqlx::Value& value);
}