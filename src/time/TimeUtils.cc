// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "time/TimeUtils.hh"
#include <libavutil/rational.h>

extern "C" {
#include "libavutil/time.h"
}

int64_t vivictpp::time::relativeTimeMicros() {
  return av_gettime_relative();
}

int64_t vivictpp::time::relativeTimeMillis() {
  return av_gettime_relative() / 1000;
}

int64_t vivictpp::time::toMicros(double seconds) {
  return static_cast<int64_t>(seconds * 1000000);
}

int64_t vivictpp::time::toMillis(int64_t micros) {
  return micros / 1000;
}

std::string vivictpp::time::formatTime(double pts, bool includeMillis) {
  int millis = static_cast<int>(pts * 1000);
  int hours = millis / (3600000);
  int minutes = (millis / 60000) % 60;
  int seconds = (millis / 1000) % 60;
  millis = millis % 1000;
  char buff[32];
  if (includeMillis) {
    if (hours > 0) {
      snprintf(buff, 32, "%02d:%02d:%02d.%03d", hours, minutes, seconds, millis);
    } else {
      snprintf(buff, 32, "%02d:%02d.%03d", minutes, seconds, millis);
    }
  } else {
    if (hours > 0) {
      snprintf(buff, 32, "%02d:%02d:%02d", hours, minutes, seconds);
    } else {
      snprintf(buff, 32, "%02d:%02d", minutes, seconds);
    }
  }
  std::string result(buff);
  return result;
}

std::string vivictpp::time::formatTime(vivictpp::time::Time pts, bool includeMillis) {
  return formatTime(pts * av_q2d(vivictpp::time::TIME_BASE_Q), includeMillis);
}
