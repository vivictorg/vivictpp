// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef AVSYNC_HH
#define AVSYNC_HH

#include <cstdint>

class AVSync {
public:
  AVSync():
    t0(0) {
  }
  void playbackStart(int64_t ptsMicros);
  int64_t clock(); // playback time micros
  int64_t diffMicros(int64_t ptsMicros);
  
private:
  int64_t t0;
};


#endif
