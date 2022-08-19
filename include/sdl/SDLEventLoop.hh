// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef SDL_EVENT_LOOP_H_
#define SDL_EVENT_LOOP_H_

#include "EventLoop.hh"
#include "sdl/SDLUtils.hh"
#include <atomic>
#include "spdlog/spdlog.h"
#include "ui/VivictPPUI.hh"
#include "ui/ScreenOutput.hh"
#include "VideoMetadata.hh"
#include "SourceConfig.hh"
#include <vector>
#include <map>

namespace vivictpp {
namespace sdl {

struct MouseState {
  bool button{false};
  int64_t buttonTime;
  bool dragging{false};
};

struct CustomEvent {
  CustomEvent(uint32_t type, std::string name):
    type(type), name(name) {};
  virtual ~CustomEvent() {};
  uint32_t type;
  std::string name;
};

struct SeekFinishedEvent: CustomEvent {
  SeekFinishedEvent(const CustomEvent &prototype, vivictpp::time::Time seekedPos):
    CustomEvent(prototype),
    seekedPos(seekedPos) {};
  vivictpp::time::Time seekedPos;
};

class SDLEventLoop : public vivictpp::ui::VivictPPUI {
public:
  SDLEventLoop(std::vector<SourceConfig> sourceConfigs);
  ~SDLEventLoop() = default;

  void scheduleAdvanceFrame(int delay) override;
  void scheduleRefreshDisplay(int delay) override;
  void scheduleQueueAudio(int delay) override;
  void scheduleFade(int delay) override;
  void scheduleSeekFinished(vivictpp::time::Time pts) override;
  void clearAdvanceFrame() override;
  void start(EventListener &eventListener) override;
  void stop() override;

  void displayFrame(const std::array<vivictpp::libav::Frame, 2> &frames,
                    const vivictpp::ui::DisplayState &displayState) override {
    screenOutput.displayFrame(frames, displayState);
  }
  int getWidth() override {
    return screenOutput.getWidth();
  }
  int getHeight() override {
    return screenOutput.getWidth();
  }
  void setFullscreen(bool fullscreen) override {
    screenOutput.setFullscreen(fullscreen);
  }
  void setLeftMetadata(const VideoMetadata &metadata) override {
    screenOutput.setLeftMetadata(metadata);
  }
  void setRightMetadata(const VideoMetadata &metadata) override {
    screenOutput.setRightMetadata(metadata);
  }
 private:
  void scheduleEvent(const CustomEvent &event, const int delay);
  void scheduleEvent(CustomEvent *event, const int delay);
  bool isCustomEvent(const SDL_Event &event);
  void handleCustomEvent(const SDL_Event &event, EventListener &eventListener);
 private:
  SDLInitializer sdlInitializer;
  vivictpp::ui::ScreenOutput screenOutput;
  std::atomic<bool> quit;
  MouseState mouseState;
  const CustomEvent refreshEventType;
  const CustomEvent advanceFrameEventType;
  const CustomEvent checkMouseDragEventType;
  const CustomEvent queueAudioEventType;
  const CustomEvent fadeEventType;
  const CustomEvent seekFinishedEventType;
  SDL_TimerID advanceFrameTimerId;
  std::shared_ptr<spdlog::logger> logger;
};

}  // ui
}  // vivictpp

#endif  // SDL_EVENT_LOOP_H_
