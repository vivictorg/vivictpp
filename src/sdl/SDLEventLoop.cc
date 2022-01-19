// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "sdl/SDLEventLoop.hh"

extern "C" {
#include <SDL.h>
}

#include "logging/Logging.hh"
#include "TimeUtils.hh"

vivictpp::sdl::SDLEventLoop::SDLEventLoop(std::vector<SourceConfig> sourceConfigs) :
  screenOutput(sourceConfigs),
  quit(false),
  refreshEventType(SDL_RegisterEvents(5), "refresh"),
  advanceFrameEventType(refreshEventType.type + 1, "advanceFrame"),
  checkMouseDragEventType(advanceFrameEventType.type + 1, "checkMouseDrag"),
  queueAudioEventType(checkMouseDragEventType.type + 1, "queueAudio"),
  fadeEventType(queueAudioEventType.type + 1, "fade"),
  advanceFrameTimerId(0),
  logger(vivictpp::logging::getOrCreateLogger("SDLEventLoop")){
}


static auto staticLogger = vivictpp::logging::getOrCreateLogger("SDLEventLoop");

static Uint32 scheduleEventCallback(Uint32 interval, void *param) {
  auto eventData = static_cast<vivictpp::sdl::CustomEvent *>(param);
  SDL_Event event;
  event.type = eventData->type;
  staticLogger->debug("SDLEventLoop::scheduleEventCallback eventType={},  interval={}", eventData->name, interval);
  SDL_PushEvent(&event);
  delete eventData;
  return 0;
}

void vivictpp::sdl::SDLEventLoop::scheduleEvent(const vivictpp::sdl::CustomEvent &eventType, const int delay) {
  logger->debug("SDLEventLoop::scheduleEvent eventType={} delay={}", eventType.name, delay);
  auto eventData = new CustomEvent(eventType);
  if (delay == 0) {
    scheduleEventCallback(0, static_cast<void *>(eventData));
  } else {
    SDL_TimerID ret = SDL_AddTimer(delay, scheduleEventCallback, static_cast<void *>(eventData));
    if (ret == 0) {
      throw std::runtime_error("Could not add timer: " +
                               std::string(SDL_GetError()));
    }
    if (eventType == advanceFrameEventType) {
      advanceFrameTimerId = ret;
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

void vivictpp::sdl::SDLEventLoop::scheduleFade(int delay) {
    logger->trace("scheduleFade");
  scheduleEvent(fadeEventType, delay);
}

void vivictpp::sdl::SDLEventLoop::clearAdvanceFrame() {
  logger->debug("SDLEventLoop::clearAdvanceFrame()");
  SDL_RemoveTimer(advanceFrameTimerId);
  SDL_PumpEvents();
  SDL_FlushEvent(advanceFrameEventType.type);
}

void vivictpp::sdl::SDLEventLoop::start(EventListener &eventListener) {
  logger->debug("SDLEventLoop::start");
  SDL_Event event;
  while (!quit.load()) {
    while (SDL_WaitEventTimeout(&event, 100) && !quit.load()) {
      logger->trace("Recieved event type={}", event.type);
      if (event.type == refreshEventType.type) {
        eventListener.refreshDisplay();
      } else if (event.type == advanceFrameEventType.type) {
        eventListener.advanceFrame();
      } else if (event.type == checkMouseDragEventType.type) {
        if (mouseState.button && !mouseState.dragging &&
            mouseState.buttonTime != 0 &&
            vivictpp::util::relativeTimeMillis() - mouseState.buttonTime > 190) {
          mouseState.dragging = true;
          screenOutput.setCursorHand();
        }
      } else if(event.type == queueAudioEventType.type) {
        eventListener.queueAudio();
      } else if(event.type == fadeEventType.type) {
        eventListener.fade();
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
          SDL_MouseButtonEvent mouseEvent = event.button;
          if (!mouseState.dragging) {
            eventListener.mouseClick(mouseEvent.x, mouseEvent.y);
          } else {
            screenOutput.setCursorDefault();
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
  }
  logger->debug("SDLEventLoop finished");
}

void vivictpp::sdl::SDLEventLoop::stop() {
  logger->debug("stopping...");
  quit.store(true);
}
