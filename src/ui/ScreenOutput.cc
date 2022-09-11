// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ui/ScreenOutput.hh"

#include "spdlog/spdlog.h"
#include "fmt/core.h"
#include "VideoMetadata.hh"
#include "Resolution.hh"
#include "ui/DisplayState.hh"
#include "ui/FrameMetadataBox.hh"
#include "sdl/SDLUtils.hh"
#include "ui/Ui.hh"

#include <cstring>
#include <exception>
#include <cmath>
#include <stdexcept>
#include <memory>

const int splashWidth = 640;
const int splashHeight = 480;

std::vector<vivictpp::vmaf::VmafLog> vmafLogs(const std::vector<SourceConfig> &sourceConfigs) {
  std::vector<vivictpp::vmaf::VmafLog> vmafLogs;
  for (auto sourceConfig: sourceConfigs) {
    vmafLogs.push_back(sourceConfig.vmafLog);
  }
  return vmafLogs;
}

vivictpp::ui::ScreenOutput::ScreenOutput(std::vector<SourceConfig> sourceConfigs)
  : sourceConfigs(sourceConfigs),
    targetResolution(splashWidth, splashHeight),
    width(splashWidth),
    height(splashHeight),
    screen(vivictpp::sdl::createWindow(width, height)),
    renderer(vivictpp::sdl::createRenderer(screen.get())),
    handCursor(vivictpp::sdl::createHandCursor()),
    panCursor(vivictpp::sdl::createPanCursor()),
    defaultCursor(SDL_GetCursor()),
    timeTextBox(Position::TOP_CENTER, {std::make_shared<TextBox>("00:00:00", "FreeMono", 24)}),
    leftMetaDisplay(Position::TOP_LEFT, true,
                    [](const DisplayState &displayState) -> const VideoMetadata& { return displayState.leftVideoMetadata; },
                    [](const DisplayState &displayState) { return displayState.leftFrame.metadata(); }),
    rightMetaDisplay(Position::TOP_RIGHT, false,
                     [](const DisplayState &displayState)  -> const VideoMetadata& { return displayState.rightVideoMetadata; },
                     [](const DisplayState &displayState) { return displayState.rightFrame.metadata(); }),
    vmafGraph(vmafLogs(sourceConfigs), 1.0f, 0.3f),
    seekBar(Margin{0,50,20,50}),
    logger(vivictpp::logging::getOrCreateLogger("ScreenOutput")) {
//  renderSplash();
}

vivictpp::ui::ScreenOutput::~ScreenOutput() {
}

Resolution vivictpp::ui::ScreenOutput::getTargetResolution(const VideoMetadata &leftVideoMetadata,
                                                           const VideoMetadata &rightVideoMetadata) {
   if (rightVideoMetadata.empty() || leftVideoMetadata.filteredResolution.w > rightVideoMetadata.filteredResolution.w) {
     return leftVideoMetadata.filteredResolution;
   }
   return rightVideoMetadata.filteredResolution;
}

void vivictpp::ui::ScreenOutput::initialize(const DisplayState &displayState) {
  setSize(getTargetResolution(displayState.leftVideoMetadata, displayState.rightVideoMetadata));
}

void vivictpp::ui::ScreenOutput::setSize(Resolution targetResolution) {
  this->targetResolution = targetResolution;
  width = std::max(targetResolution.w, width);
  height = std::max(targetResolution.h, height);
  SDL_SetWindowSize(screen.get(), width, height);
}

void vivictpp::ui::ScreenOutput::setCursorHand() { SDL_SetCursor(handCursor.get()); }

void vivictpp::ui::ScreenOutput::setCursorPan() { SDL_SetCursor(panCursor.get()); }

void vivictpp::ui::ScreenOutput::setCursorDefault() { SDL_SetCursor(defaultCursor); }

void vivictpp::ui::ScreenOutput::setFullscreen(bool fullscreen) {
  if (fullscreen) {
    SDL_SetWindowFullscreen(screen.get(), SDL_WINDOW_FULLSCREEN);
  } else {
    SDL_SetWindowFullscreen(screen.get(), 0);
  }
}

void vivictpp::ui::ScreenOutput::drawTime(const DisplayState &displayState) {
  timeTextBox.getComponent<TextBox>(0).setText(displayState.timeStr);
  timeTextBox.render(displayState, renderer.get());
}

void vivictpp::ui::debugRectangle(std::string msg, const SDL_Rect &rect) {
  spdlog::info("{}: x={},y={},w={},h={}", msg, rect.x,
                rect.y, rect.w, rect.h);
}

void vivictpp::ui::setRectangle(SDL_Rect &rect, int x, int y, int w, int h) {
  rect.x = x;
  rect.y = y;
  rect.w = w;
  rect.h = h;
}

void vivictpp::ui::ScreenOutput::renderSplash() {
  DisplayState displayState;
  SDL_SetRenderDrawColor(renderer.get(), 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer.get());
  splashText.render(displayState, renderer.get(), 0, 0);
  SDL_RenderPresent(renderer.get());
}

void vivictpp::ui::ScreenOutput::displayFrame(
    const DisplayState &displayState) {
  if (videoMetadataVersion != displayState.videoMetadataVersion) {
    initialize(displayState);
    videoMetadataVersion = displayState.videoMetadataVersion;
  }
  
//  if (!initialized && !displayState.leftFrame.empty()) {
//    logger->debug("vivictpp::ui::ScreenOutput::displayFrame initializing");
//    initialize(displayState);
//  }

  SDL_SetRenderDrawColor(renderer.get(), 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer.get());

  videoDisplay.render(displayState, renderer.get(), 0, 0);

  if (displayState.displayTime) {
    drawTime(displayState);
  }

  leftMetaDisplay.update(displayState);
  if (!displayState.splitScreenDisabled) {
    rightMetaDisplay.update(displayState);
  }

  leftMetaDisplay.render(displayState, renderer.get(), 0, 0);
  if (!displayState.splitScreenDisabled) {
    rightMetaDisplay.render(displayState, renderer.get(), 0, 0);
  }

  if (displayState.displayPlot && !vmafGraph.empty()) {
    vmafGraph.render(renderer.get(), displayState.pts, displayState.leftVideoMetadata.startTime,
                     displayState.leftVideoMetadata.duration);
  }
   if (displayState.seekBar.visible) {
    seekBar.setState(displayState.seekBar);
    int y = height - seekBar.preferredHeight();
    seekBar.render(displayState, renderer.get(), 0, y);
  }
  SDL_RenderPresent(renderer.get());
}

const vivictpp::ui::MouseClicked vivictpp::ui::ScreenOutput::getClickTarget(int x, int y) {
  // TODO: Handle plot
  if (seekBar.getBox().contains(x,y)) {
    return {"seekbar", x, y, std::ref<vivictpp::ui::Component>(seekBar)};
  }
  else {
    return {"default", x, y, std::ref<vivictpp::ui::Component>(timeTextBox)}; // TODO: Use actual root component
  }
}
