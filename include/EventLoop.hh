// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef EVENT_LOOP_H_
#define EVENT_LOOP_H_

#include "EventListener.hh"

class EventScheduler {
public:
  virtual ~EventScheduler() = default;
  virtual void scheduleAdvanceFrame(int delay) = 0;
  virtual void scheduleRefreshDisplay(int delay) = 0;
  virtual void scheduleQueueAudio(int delay) = 0;
};

class EventLoop: public EventScheduler {
 public:
  virtual ~EventLoop() = default;
  virtual void start(EventListener &eventListener) = 0;
  virtual void stop() = 0;
};

#endif  // EVENT_LOOP_H
