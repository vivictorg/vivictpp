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

class CustomEvent {
public:
  CustomEvent(unsigned int type, std::string name):
    type(type),
    name(name) {};
  CustomEvent(const CustomEvent &other) = default;
  bool operator==(const CustomEvent other) const { return this->type == other.type; };
public:
  const unsigned int type;
  const std::string name;
};


class SDLEventLoop : public vivictpp::ui::VivictPPUI {
public:
  SDLEventLoop(std::vector<SourceConfig> sourceConfigs);
  ~SDLEventLoop() = default;

  void scheduleAdvanceFrame(int delay) override;
  void scheduleRefreshDisplay(int delay) override;
  void scheduleQueueAudio(int delay) override;
  void scheduleFade(int delay) override;
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
  void scheduleEvent(const CustomEvent &eventType, const int delay);
 private:
  SDLInitializer sdlInitializer;
  vivictpp::ui::ScreenOutput screenOutput;
  std::atomic<bool> quit;
  MouseState mouseState;
  CustomEvent refreshEventType;
  CustomEvent advanceFrameEventType;
  CustomEvent checkMouseDragEventType;
  CustomEvent queueAudioEventType;
  CustomEvent fadeEventType;

  SDL_TimerID advanceFrameTimerId;
  std::shared_ptr<spdlog::logger> logger;
};

}  // ui
}  // vivictpp

#endif  // SDL_EVENT_LOOP_H_
