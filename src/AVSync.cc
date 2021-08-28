// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "AVSync.hh"

#include "TimeUtils.hh"

void AVSync::playbackStart(int64_t ptsMicros) {
  t0 = vivictpp::util::relativeTimeMicros() - ptsMicros;
}

int64_t AVSync::clock() {
  int64_t t = vivictpp::util::relativeTimeMicros();
  return  t - t0;
}

int64_t AVSync::diffMicros(int64_t ptsMicros) {
  return ptsMicros - (vivictpp::util::relativeTimeMicros() - t0);
}
