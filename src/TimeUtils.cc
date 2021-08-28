// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "TimeUtils.hh"

extern "C" {
#include "libavutil/time.h"
}

int64_t vivictpp::util::relativeTimeMicros() {
  return av_gettime_relative();
}

int64_t vivictpp::util::relativeTimeMillis() {
  return av_gettime_relative() / 1000;
}

int64_t vivictpp::util::toMicros(double seconds) {
  return static_cast<int64_t>(seconds * 1000000);
}

int64_t vivictpp::util::toMillis(int64_t micros) {
  return micros / 1000;
}

std::string vivictpp::util::formatTime(double pts) {
  int millis = static_cast<int>(pts * 1000);
  int hours = millis / (3600000);
  int minutes = (millis / 60000) % 60;
  int seconds = (millis / 1000) % 60;
  millis = millis % 1000;
  char buff[32];
  if (hours > 0) {
    snprintf(buff, 32, "%02d:%02d:%02d.%03d", hours, minutes, seconds, millis);
  } else {
     snprintf(buff, 32, "%02d:%02d.%03d", minutes, seconds, millis);
  }
  std::string result(buff);
  return result;
}

