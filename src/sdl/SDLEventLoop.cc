// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "sdl/SDLEventLoop.hh"
#include "SDL_events.h"
#include "SDL_video.h"
#include "ui/Events.hh"
#include <memory>

extern "C" {
#include <SDL.h>
}

#include "logging/Logging.hh"
#include "time/TimeUtils.hh"

vivictpp::sdl::SDLEventLoop::SDLEventLoop(std::vector<SourceConfig> sourceConfigs) :
  screenOutput(sourceConfigs),
  quit(false),
  refreshEventType(SDL_RegisterEvents(6), "refresh"),
  advanceFrameEventType(refreshEventType.type + 1, "advanceFrame"),
  checkMouseDragEventType(advanceFrameEventType.type + 1, "checkMouseDrag"),
  queueAudioEventType(checkMouseDragEventType.type + 1, "queueAudio"),
  fadeEventType(queueAudioEventType.type + 1, "fade"),
  seekFinishedEventType(fadeEventType.type + 1, "seekFinished"),
  advanceFrameTimerId(0),
  logger(vivictpp::logging::getOrCreateLogger("SDLEventLoop")){
}


static auto staticLogger = vivictpp::logging::getOrCreateLogger("SDLEventLoop");

static Uint32 scheduleEventCallback(Uint32 interval, void *param) {
  auto eventData = static_cast<vivictpp::sdl::CustomEvent *>(param);
  SDL_Event event;
  SDL_zero(event);
  event.type = eventData->type;
  event.user.code = eventData->type;
  event.user.data1 = eventData;
  staticLogger->debug("SDLEventLoop::scheduleEventCallback eventType={},  interval={}", eventData->name, interval);
  SDL_PushEvent(&event);
  return 0;
}

void vivictpp::sdl::SDLEventLoop::scheduleEvent(const vivictpp::sdl::CustomEvent &event, const int delay) {
  this->scheduleEvent(new CustomEvent(event), delay);
}

void vivictpp::sdl::SDLEventLoop::scheduleEvent(vivictpp::sdl::CustomEvent *event, const int delay) {
  logger->debug("SDLEventLoop::scheduleEvent eventType={} delay={}", event->name, delay);
//  auto eventData = new CustomEvent(eventType);
  if (delay == 0) {
    scheduleEventCallback(0, static_cast<void *>(event));
  } else {
    bool saveTimerId = event->type == advanceFrameEventType.type;
    SDL_TimerID ret = SDL_AddTimer(delay, scheduleEventCallback, static_cast<void *>(event));
    if (ret == 0) {
      throw std::runtime_error("Could not add timer: " +
                               std::string(SDL_GetError()));
    }
    if (saveTimerId) {
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

void vivictpp::sdl::SDLEventLoop::scheduleSeekFinished(vivictpp::time::Time seekedPos) {
    logger->trace("scheduleFade");
    SeekFinishedEvent *event = new SeekFinishedEvent(seekFinishedEventType, seekedPos);
    scheduleEvent(event, 0);
}

void vivictpp::sdl::SDLEventLoop::clearAdvanceFrame() {
  logger->debug("SDLEventLoop::clearAdvanceFrame()");
  SDL_RemoveTimer(advanceFrameTimerId);
  SDL_PumpEvents();
  SDL_FlushEvent(advanceFrameEventType.type);

}

vivictpp::KeyModifiers getKeyModifiers() {
  SDL_Keymod modState = SDL_GetModState();
  return {!!(modState & KMOD_SHIFT), !!(modState & KMOD_CTRL), !!(modState & KMOD_ALT)};
}

bool vivictpp::sdl::SDLEventLoop::isCustomEvent(const SDL_Event &event) {
  return event.type >= refreshEventType.type && event.type <= seekFinishedEventType.type;
}

void vivictpp::sdl::SDLEventLoop::handleCustomEvent(const SDL_Event &event, EventListener &eventListener) {
  // Use shared_ptr to ensure delete after function 
  std::shared_ptr<CustomEvent> customEvent(static_cast<vivictpp::sdl::CustomEvent *>(event.user.data1));
  if (event.type == refreshEventType.type) {
    eventListener.refreshDisplay();
  } else if (event.type == advanceFrameEventType.type) {
    eventListener.advanceFrame();
  } else if (event.type == checkMouseDragEventType.type) {
    if (mouseState.button && !mouseState.dragging &&
        mouseState.buttonTime != 0 &&
        vivictpp::time::relativeTimeMillis() - mouseState.buttonTime > 190) {
      mouseState.dragging = true;
      if (mouseState.mouseClicked.has_value() && mouseState.mouseClicked.value().target == "seekbar") {
        screenOutput.setCursorHand();
      } else {
        screenOutput.setCursorPan();
      }
      vivictpp::ui::MouseDragStarted mouseDragStarted = {
        mouseState.mouseClicked.value().target,
        mouseState.mouseClicked.value().x,
        mouseState.mouseClicked.value().y,
        mouseState.mouseClicked.value().component
      };
      eventListener.mouseDragStarted(mouseDragStarted);
    }
  } else if(event.type == queueAudioEventType.type) {
    eventListener.queueAudio();
  } else if(event.type == fadeEventType.type) {
    eventListener.fade();
  } else if(event.type == seekFinishedEventType.type) {
    std::shared_ptr<SeekFinishedEvent> seekFinished = std::dynamic_pointer_cast<SeekFinishedEvent>(customEvent);
    eventListener.seekFinished(seekFinished->seekedPos);
  }
}

void vivictpp::sdl::SDLEventLoop::start(EventListener &eventListener) {
  logger->debug("SDLEventLoop::start");
  SDL_Event event;
  while (!quit.load()) {
    while (SDL_WaitEventTimeout(&event, 100) && !quit.load()) {
      logger->trace("Recieved event type={}", event.type);
      if (isCustomEvent(event)) {
        handleCustomEvent(event, eventListener);
      } else {
        switch (event.type) {
        case SDL_QUIT:
          quit.store(true);
          break;
        case SDL_MOUSEMOTION: {
          SDL_MouseMotionEvent mouseEvent = event.motion;
          if (!mouseState.dragging) {
            eventListener.mouseMotion(mouseEvent.x, mouseEvent.y);
          } else {
            vivictpp::ui::MouseDragged mouseDragged = {
              mouseState.mouseClicked.value().target,
              mouseEvent.x,
              mouseEvent.y,
              mouseEvent.xrel,
              mouseEvent.yrel,
              mouseState.mouseClicked.value().component
            };
            eventListener.mouseDrag(mouseDragged);
          }
        } break;
        case SDL_MOUSEWHEEL:
          eventListener.mouseWheel(event.wheel.x, event.wheel.y);
          break;
        case SDL_MOUSEBUTTONDOWN: {
          SDL_MouseButtonEvent mouseEvent = event.button;
          mouseState.button = true;
          mouseState.buttonTime = vivictpp::time::relativeTimeMillis();
          mouseState.mouseClicked.emplace(screenOutput.getClickTarget(mouseEvent.x, mouseEvent.y));
          scheduleEvent(checkMouseDragEventType, 200);
        } break;
        case SDL_MOUSEBUTTONUP: {
          if (!mouseState.dragging) {
            eventListener.mouseClick(mouseState.mouseClicked.value());
          } else {
            screenOutput.setCursorDefault();
            vivictpp::ui::MouseDragStopped mouseDragStopped = {
              mouseState.mouseClicked.value().target,
              mouseState.mouseClicked.value().component
            };
            eventListener.mouseDragStopped(mouseDragStopped);
          }
          mouseState.button = false;
          mouseState.dragging = false;
          mouseState.mouseClicked.reset();
          mouseState.buttonTime = 0;
        } break;
        case SDL_KEYDOWN: {
          SDL_KeyboardEvent kbe = event.key;
          eventListener.keyPressed(std::string(SDL_GetKeyName(kbe.keysym.sym)),
                                   getKeyModifiers());
        } break;
        case SDL_WINDOWEVENT: {
          if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
            logger->debug("Window {} size changed to {}x{}",
                          event.window.windowID, event.window.data1,
                          event.window.data2);
            eventListener.refreshDisplay();
          }
        } break;
        }
      }
    }
  }
  logger->debug("SDLEventLoop finished");
}

void vivictpp::sdl::SDLEventLoop::stop() {
  logger->debug("stopping...");
  quit.store(true);
}

