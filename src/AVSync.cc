// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "AVSync.hh"

#include "time/TimeUtils.hh"

void vivictpp::AVSync::playbackStart(vivictpp::time::Time ptsMicros) {
  t0 = vivictpp::time::relativeTimeMicros() - ptsMicros;
}

int64_t vivictpp::AVSync::clock() {
  int64_t t = vivictpp::time::relativeTimeMicros();
  return  t - t0;
}

int64_t vivictpp::AVSync::diffMicros(vivictpp::time::Time ptsMicros) {
  return ptsMicros - (vivictpp::time::relativeTimeMicros() - t0);
}
