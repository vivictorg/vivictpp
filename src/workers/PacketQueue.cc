// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "workers/PacketQueue.hh"

vivictpp::workers::PacketQueue::PacketQueue(std::size_t maxSize)
    : maxSize(maxSize) {}

bool vivictpp::workers::PacketQueue::add(AVPacket *pkt) {
  bool wasEmpty = false;
  {
    std::unique_lock<std::mutex> lock(mutex);
    wasEmpty = _queue.size() == 0;
    if (_queue.size() == maxSize) {
      return false;
    }
    _queue.push(vivictpp::libav::Packet(pkt));
  }
  if (wasEmpty) {
    conditionVariable.notify_all();
  }
  return true;
}

vivictpp::libav::Packet vivictpp::workers::PacketQueue::remove() {
  bool wasFull = false;
  vivictpp::libav::Packet result;
  {
    std::unique_lock<std::mutex> lock(mutex);
    if (_queue.size() > 0) {
      wasFull = _queue.size() == maxSize;
      result = _queue.front();
      _queue.pop();
    }
  }
  if (wasFull) {
    conditionVariable.notify_all();
  }
  return result;
}

bool vivictpp::workers::PacketQueue::waitForNotFull(
    const std::chrono::milliseconds &relTime) {
  std::unique_lock<std::mutex> lock(mutex);
  return conditionVariable.wait_for(lock, relTime,
                                    [&] { return _queue.size() < maxSize; });
}

bool vivictpp::workers::PacketQueue::waitForNotEmpty(
    const std::chrono::milliseconds &relTime) {
  std::unique_lock<std::mutex> lock(mutex);
  return conditionVariable.wait_for(lock, relTime,
                                    [&] { return _queue.size() > 0; });
}

void vivictpp::workers::PacketQueue::clear() {
  bool wasFull = false;
  {
    std::unique_lock<std::mutex> lock(mutex);
    wasFull = _queue.size() == maxSize;
    while (!_queue.empty()) {
      _queue.pop();
    }
  }
  if (wasFull) {
    conditionVariable.notify_all();
  }
}
