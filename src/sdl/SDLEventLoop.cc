// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "sdl/SDLEventLoop.hh"

extern "C" {
#include <SDL.h>
}

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "TimeUtils.hh"

struct EventData {
  int type;
};

vivictpp::sdl::SDLEventLoop::SDLEventLoop() :
  quit(false),
  refreshEventType(SDL_RegisterEvents(4)),
  advanceFrameEventType(refreshEventType + 1),
  checkMouseDragEventType(advanceFrameEventType + 1),
  queueAudioEventType(checkMouseDragEventType + 1),
  logger(spdlog::stdout_color_mt("SDLEventLoop")){
}

static Uint32 scheduleEventCallback(Uint32 interval, void *param) {
    spdlog::trace("scheduleEventCallback interval={}", interval);
  auto eventData = static_cast<EventData *>(param);
  SDL_Event event;
  event.type = eventData->type;
  SDL_PushEvent(&event);
  delete eventData;
  return 0;
}

void scheduleEvent(int eventType, int delay) {
    spdlog::trace("scheduleEvent eventType={} delay={}", eventType, delay);
  auto eventData = new EventData();
  eventData->type = eventType;
  if (delay == 0) {
    scheduleEventCallback(0, static_cast<void *>(eventData));
  } else {
    int ret = SDL_AddTimer(delay, scheduleEventCallback, static_cast<void *>(eventData));
    if (ret == 0) {
      throw std::runtime_error("Could not add timer: " +
                               std::string(SDL_GetError()));
    }
  }
}

void vivictpp::sdl::SDLEventLoop::scheduleAdvanceFrame(int delay) {
    logger->trace("scheduleAdvanceFrame");
  scheduleEvent(advanceFrameEventType, delay);
}

void vivictpp::sdl::SDLEventLoop::scheduleRefreshDisplay(int delay) {
    logger->trace("scheduleRefreshDisplay");
  scheduleEvent(refreshEventType, delay);
}

void vivictpp::sdl::SDLEventLoop::scheduleQueueAudio(int delay) {
    logger->trace("scheduleQueueAudio");
  scheduleEvent(queueAudioEventType, delay);
}

void vivictpp::sdl::SDLEventLoop::start(EventListener &eventListener) {
  logger->debug("SDLEventLoop::start");
  SDL_Event event;
  while (!quit.load()) {
    while (SDL_WaitEventTimeout(&event, 100) && !quit.load()) {
      logger->trace("Recieved event type={}", event.type);
      if (event.type == refreshEventType) {
        eventListener.refreshDisplay();
      } else if (event.type == advanceFrameEventType) {
        eventListener.advanceFrame();
      } else if (event.type == checkMouseDragEventType) {
        if (mouseState.button && !mouseState.dragging &&
            mouseState.buttonTime != 0 &&
            vivictpp::util::relativeTimeMillis() - mouseState.buttonTime > 190) {
          mouseState.dragging = true;
          eventListener.mouseDragStart();
        }
      } else if(event.type == queueAudioEventType) {
        eventListener.queueAudio();
      } else
        switch (event.type) {
        case SDL_QUIT:
          quit.store(true);
          break;
        case SDL_MOUSEMOTION: {
          SDL_MouseMotionEvent mouseEvent = event.motion;
          if (!mouseState.dragging) {
            eventListener.mouseMotion(mouseEvent.x, mouseEvent.y);
          } else {
            eventListener.mouseDrag(mouseEvent.xrel, mouseEvent.yrel);
          }
        } break;
        case SDL_MOUSEWHEEL:
          eventListener.mouseWheel(event.wheel.x, event.wheel.y);
          break;
        case SDL_MOUSEBUTTONDOWN: {
          mouseState.button = true;
          mouseState.buttonTime = vivictpp::util::relativeTimeMillis();
          scheduleEvent(checkMouseDragEventType, 200);
        } break;
        case SDL_MOUSEBUTTONUP: {
          if (!mouseState.dragging) {
            eventListener.mouseClick();
          } else {
            eventListener.mouseDragEnd();
          }
          mouseState.button = false;
          mouseState.dragging = false;
          mouseState.buttonTime = 0;
        } break;
        case SDL_KEYDOWN: {
          SDL_KeyboardEvent kbe = event.key;
          eventListener.keyPressed(std::string(SDL_GetKeyName(kbe.keysym.sym)));
        } break;
        case SDL_WINDOWEVENT: {
          if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
            logger->debug("Window {} size changed to {}x{}",
                         event.window.windowID, event.window.data1,
                         event.window.data2);
          }
        } break;
        }
    }
    logger->debug("SDLEventLoop finished");
  }
}

void vivictpp::sdl::SDLEventLoop::stop() {
    logger->debug("stopping...");
  quit.store(true);
}
