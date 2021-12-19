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

namespace vivictpp {
namespace sdl {

struct MouseState {
  bool button{false};
  int64_t buttonTime;
  bool dragging{false};
};


class SDLEventLoop : public vivictpp::ui::VivictPPUI {
public:
  SDLEventLoop(std::vector<SourceConfig> sourceConfigs);
  ~SDLEventLoop() = default;

  void scheduleAdvanceFrame(int delay) override;
  void scheduleRefreshDisplay(int delay) override;
  void scheduleQueueAudio(int delay) override;
  void scheduleFade(int delay) override;
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
  SDLInitializer sdlInitializer;
  vivictpp::ui::ScreenOutput screenOutput;
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
