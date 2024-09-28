// SPDX-FileCopyrightText: 2020 Sveriges Television AB
// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef TIME_TIMEUTILS_HH
#define TIME_TIMEUTILS_HH

#include "time/Time.hh"
#include <string>
// #include "spdlog/spdlog.h"
#include "spdlog/fmt/fmt.h"

namespace vivictpp {
namespace time {

int64_t relativeTimeMicros();

int64_t relativeTimeMillis();

int64_t toMicros(double seconds);

int64_t toMillis(int64_t micros);

std::string formatTime(double pts, bool includeMillis = true);

std::string formatTime(vivictpp::time::Time pts, bool includeMillis = true);

} // namespace time
} // namespace vivictpp

template <>
struct fmt::formatter<vivictpp::time::Time> : fmt::formatter<std::string> {
  auto format(vivictpp::time::Time time,
              format_context &ctx) -> decltype(ctx.out()) {
    return format_to(ctx.out(), "{}", vivictpp::time::formatTime(time));
  }
};

#endif // TIME_TIMEUTILS_HH
