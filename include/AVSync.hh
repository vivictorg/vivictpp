// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef AVSYNC_HH
#define AVSYNC_HH

#include <cstdint>

#include "time/Time.hh"

namespace vivictpp {

class AVSync {
public:
  AVSync():
    t0(0) {
  }
  void playbackStart(vivictpp::time::Time ptsMicros);
  int64_t clock(); // playback time micros
  int64_t diffMicros(vivictpp::time::Time ptsMicros);

private:
  int64_t t0;
};

};  // namespace vivictpp

#endif
