// SPDX-FileCopyrightText: 2020 Sveriges Television AB
// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FRAMEBUFFER_HH_
#define FRAMEBUFFER_HH_

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <unistd.h>
}

#include <iostream>
#include <limits>
#include <mutex>
#include <vector>
#include <condition_variable>
#include <chrono>

#include "workers/QueuePointer.hh"
#include "libav/Frame.hh"
#include "logging/Logging.hh"
#include "time/Time.hh"

namespace vivictpp {
namespace workers {

/*
  Circular buffer for storing decoded frames. Stores index to the head and tail
 position, as well as the current read index. If the read index gets too close
to the head, size is decreased to make room for new frames. Pts for each frame
is stored in a separate array.

It is assumed that there is only a single reader thread and a single writer thread.

A mutex is used to ensure consistency

 */

class FrameBuffer {
public:
  FrameBuffer(int _maxSize);
  ~FrameBuffer() = default;
  vivictpp::libav::Frame first();
  void write(vivictpp::libav::Frame frame, vivictpp::time::Time pts);
  vivictpp::time::Time nextPts();
  vivictpp::time::Time previousPts();
  int stepForward(vivictpp::time::Time pts);
  void stepBackward(vivictpp::time::Time pts);
  void drop(int n = 1);
  void dropIfFull(int n);
  int size();
  vivictpp::time::Time currentPts();
  void clear();
  bool ptsInRange(vivictpp::time::Time pts);
  vivictpp::time::Time minPts();
  vivictpp::time::Time maxPts();
  bool isFull();
  bool waitForNotFull(const std::chrono::milliseconds& relTime);
  bool isEmpty();
  const std::vector<vivictpp::time::Time> &getPtsBuffer() { return ptsBuffer; }

private:
  bool next();
  bool previous();
  QueuePointer tail() { return _writePos - _size; }
  QueuePointer head() { return _writePos - 1; }
  void _drop(int n = 1);

private:
    vivictpp::logging::Logger logger;
  std::vector<vivictpp::libav::Frame> queue;
  std::vector<vivictpp::time::Time> ptsBuffer;
  QueuePointer _writePos; // Points to first empty slot,
                          // ie one ahead of written value
  QueuePointer _cursor;
  int _size;
  int _maxSize;
  std::mutex mutex;
  std::condition_variable conditionVariable;
};
}  // namespace workers
}  // namespace vivictpp
#endif // FRAMEBUFFER_HH_

