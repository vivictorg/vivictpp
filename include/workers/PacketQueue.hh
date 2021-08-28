// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef PACKET_QUEUE_HH_
#define PACKET_QUEUE_HH_

#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <memory>
#include <functional>

extern "C" {
#include <libavformat/avformat.h>
}

#include "libav/Packet.hh"

namespace vivictpp {
namespace workers {

class PacketQueue {
public:
  PacketQueue(std::size_t maxSize);
  ~PacketQueue() = default;

  bool add(AVPacket *pkt);
  vivictpp::libav::Packet remove();
  bool waitForNotFull(const std::chrono::milliseconds& relTime);
  bool waitForNotEmpty(const std::chrono::milliseconds& relTime);
  void clear();
private:
  std::size_t maxSize;
  std::queue<vivictpp::libav::Packet> _queue;
  std::mutex mutex;
  std::condition_variable conditionVariable;
};

};  // namespace workers
};  // namespace vivictpp

#endif  // PACKET_QUEUE_HH_
