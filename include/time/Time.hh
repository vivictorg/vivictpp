// SPDX-FileCopyrightText: 2022 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef TIME_TIME_HH
#define TIME_TIME_HH

extern "C" {
#include <libavutil/avutil.h>
#include <libavutil/rational.h>
}

namespace vivictpp {
namespace time {

typedef int64_t Time;

const Time NO_TIME = AV_NOPTS_VALUE;
const int64_t TIME_BASE = AV_TIME_BASE;
const AVRational TIME_BASE_Q = {1, AV_TIME_BASE};

inline double ptsToDouble(Time pts) { return pts * av_q2d(TIME_BASE_Q); }

inline bool isNoPts(Time pts) { return pts == NO_TIME; }

inline Time seconds(int s) { return s * TIME_BASE; }

inline Time millis(int m) { return m * (TIME_BASE / 1000); }

} // namespace time
} // namespace vivictpp

#endif // TIME_TIME_HH
