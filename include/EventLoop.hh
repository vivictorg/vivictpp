// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef EVENTLOOP_HH
#define EVENTLOOP_HH

#include "EventListener.hh"
#include "time/Time.hh"

class EventScheduler {
public:
  virtual ~EventScheduler() = default;
  virtual void scheduleAdvanceFrame(int delay) = 0;
  virtual void scheduleRefreshDisplay(int delay) = 0;
  virtual void scheduleQueueAudio(int delay) = 0;
  virtual void scheduleFade(int delay) = 0;
  virtual void scheduleSeekFinished(vivictpp::time::Time pts, bool error) = 0;
  virtual void clearAdvanceFrame() = 0;
};

class EventLoop: public EventScheduler {
 public:
  virtual ~EventLoop() = default;
  virtual void start(vivictpp::EventListener &eventListener) = 0;
  virtual void stop() = 0;
};

#endif // EVENTLOOP_HH
