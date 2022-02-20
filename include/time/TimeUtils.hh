// SPDX-FileCopyrightText: 2020 Sveriges Television AB
// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef TIMEUTIL_HH_
#define TIMEUTIL_HH_

#include <string>
#include "time/Time.hh"

namespace vivictpp {
namespace time {

int64_t relativeTimeMicros();

int64_t relativeTimeMillis();

int64_t toMicros(double seconds);

int64_t toMillis(int64_t micros);

std::string formatTime(double pts);

std::string formatTime(vivictpp::time::Time pts);

}  // namespace time
}  // namespace vivictpp

#endif // TIMEUTIL_HH_
