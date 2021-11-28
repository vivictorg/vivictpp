// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef SDL_EVENT_LOOP_H_
#define SDL_EVENT_LOOP_H_

#include "EventLoop.hh"
#include "sdl/SDLUtils.hh"
#include <atomic>
#include "spdlog/spdlog.h"

namespace vivictpp {
namespace sdl {

struct MouseState {
  bool button{false};
  int64_t buttonTime;
  bool dragging{false};
};


class SDLEventLoop : public EventLoop {
 public:
  SDLEventLoop();
  ~SDLEventLoop() = default;
  void scheduleAdvanceFrame(int delay) override;
  void scheduleRefreshDisplay(int delay) override;
  void scheduleQueueAudio(int delay) override;
  void scheduleFade(int delay) override;
  void start(EventListener &eventListener) override;
  void stop() override;

 private:
  SDLInitializer sdlInitializer;
  std::atomic<bool> quit;
  MouseState mouseState;
  unsigned int refreshEventType;
  unsigned int advanceFrameEventType;
  unsigned int checkMouseDragEventType;
  unsigned int queueAudioEventType;
  unsigned int fadeEventType;
    std::shared_ptr<spdlog::logger> logger;
};

}  // ui
}  // vivictpp

#endif  // SDL_EVENT_LOOP_H_
