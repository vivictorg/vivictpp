// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ui/ScreenOutput.hh"

#include "spdlog/spdlog.h"
#include "VideoMetadata.hh"
#include "Resolution.hh"
#include "sdl/SDLUtils.hh"

#include <cstring>
#include <exception>
#include <cmath>
#include <stdexcept>

const int seekBarHeight = 10;
const int seekBarLeftMargin = 50;
const int seekBarBottomMargin = 20;
const int splashWidth = 640;
const int splashHeight = 480;

const std::string SPLASH_TEXT("____   ____._______   ____._______________________                       \n\\   \\ /   /|   \\   \\ /   /|   \\_   ___ \\__    ___/    .__         .__    \n \\   Y   / |   |\\   Y   / |   /    \\  \\/ |    |     __|  |___   __|  |___\n  \\     /  |   | \\     /  |   \\     \\____|    |    /__    __/  /__    __/\n   \\___/   |___|  \\___/   |___|\\______  /|____|       |__|        |__|   ");

Resolution getTargetResolution(const std::shared_ptr<VideoMetadata> &leftVideoMetadata,
                               const std::shared_ptr<VideoMetadata> &rightVideoMetadata){
  if ((!rightVideoMetadata) || leftVideoMetadata->width > rightVideoMetadata->width) {
    return leftVideoMetadata->resolution;
  }
  return rightVideoMetadata->resolution;
}

std::vector<vivictpp::vmaf::VmafLog> vmafLogs(const std::vector<SourceConfig> &sourceConfigs) {
  std::vector<vivictpp::vmaf::VmafLog> vmafLogs;
  for (auto sourceConfig: sourceConfigs) {
    vmafLogs.push_back(sourceConfig.vmafLog);
  }
  return vmafLogs;
}


vivictpp::ui::ScreenOutput::ScreenOutput(std::vector<SourceConfig> sourceConfigs)
  : leftVideoMetadata(nullptr),
    rightVideoMetadata(nullptr),
    sourceConfigs(sourceConfigs),
    targetResolution(splashWidth, splashHeight),
    width(splashWidth),
    height(splashHeight),
    sdlInitializer(),
    screen(vivictpp::sdl::createWindow(width, height)),
    renderer(vivictpp::sdl::createRenderer(screen.get())),
    leftTexture(nullptr, nullptr),
    rightTexture(nullptr, nullptr),
    handCursor(vivictpp::sdl::createHandCursor()),
    defaultCursor(SDL_GetCursor()),
    timeTextBox("00:00:00", "FreeMono", 24, TextBoxPosition::TOP_CENTER),
    leftMetadataBox("", "FreeMono", 16, TextBoxPosition::TOP_LEFT,
                    0,0, "Stream Info"),
    rightMetadataBox("", "FreeMono", 16, TextBoxPosition::TOP_RIGHT,
                     0,0, "Stream Info"),
    leftFrameBox("", "FreeMono", 16, TextBoxPosition::TOP_LEFT, 0, 140,
                 "Frame Info"),
    rightFrameBox("", "FreeMono", 16, TextBoxPosition::TOP_RIGHT, 0, 140,
                  "Frame Info", 120),
    frameOffsetBox("", "FreeMono", 16, TextBoxPosition::TOP_LEFT, 0, 202,
                   "Frame offset", 120),
    splashText(SPLASH_TEXT, "FreeMono", 32 , TextBoxPosition::CENTER),
    vmafGraph(vmafLogs(sourceConfigs), 1.0f, 0.3f),
    logger(vivictpp::logging::getOrCreateLogger("ScreenOutput")) {

  /*
  if (rightVideoMetadata) {
    rightTexture = vivictpp::sdl::createTexture(renderer.get(),
                                 rightVideoMetadata->width, rightVideoMetadata->height);
    rightMetadataBox.setText(rightVideoMetadata->toString());
  }
  */

  leftMetadataBox.bg = {50, 50, 50, 100};
  rightMetadataBox.bg = {50, 50, 50, 100};
  leftFrameBox.bg = {50, 50, 50, 100};
  rightFrameBox.bg = {50, 50, 50, 100};
  frameOffsetBox.bg = {50, 50, 50, 100};
  splashText.bg = {0,0,0,255};
  splashText.border = false;
  renderSplash();
}

vivictpp::ui::ScreenOutput::~ScreenOutput() {
}

Resolution vivictpp::ui::ScreenOutput::getTargetResolution() {
   if ((!rightVideoMetadata) || leftVideoMetadata->width > rightVideoMetadata->width) {
    return leftVideoMetadata->resolution;
  }
  return rightVideoMetadata->resolution;
}

void vivictpp::ui::ScreenOutput::setLeftMetadata(const VideoMetadata &metadata) {
  leftVideoMetadata.reset( new VideoMetadata(metadata));
  leftMetadataBox.setText(leftVideoMetadata->toString());
  leftTexture = vivictpp::sdl::createTexture(renderer.get(),
                              leftVideoMetadata->width, leftVideoMetadata->height);
  setSize();
}

void vivictpp::ui::ScreenOutput::setRightMetadata(const VideoMetadata &metadata) {
  rightVideoMetadata.reset( new VideoMetadata(metadata));
  rightMetadataBox.setText(rightVideoMetadata->toString());
  rightTexture = vivictpp::sdl::createTexture(renderer.get(),
                               rightVideoMetadata->width, rightVideoMetadata->height);
  setSize();
}

void vivictpp::ui::ScreenOutput::setSize() {
  targetResolution = getTargetResolution();
  width = std::max(targetResolution.w, width);
  height = std::max(targetResolution.h, height);
  SDL_SetWindowSize(screen.get(), width, height);
}

void vivictpp::ui::ScreenOutput::setCursorHand() { SDL_SetCursor(handCursor.get()); }

void vivictpp::ui::ScreenOutput::setCursorDefault() { SDL_SetCursor(defaultCursor); }

int fitToRange(int value, int min, int max) {
  return std::max(min, std::min(max, value));
}

void vivictpp::ui::ScreenOutput::setFullscreen(bool fullscreen) {
  if (fullscreen) {
    SDL_SetWindowFullscreen(screen.get(), SDL_WINDOW_FULLSCREEN);
  } else {
    SDL_SetWindowFullscreen(screen.get(), 0);
  }
}

void vivictpp::ui::ScreenOutput::drawTime(const DisplayState &displayState) {
  timeTextBox.setText(displayState.timeStr);
  timeTextBox.render(renderer.get());
}

void vivictpp::ui::ScreenOutput::calcZoomedSrcRect(const DisplayState &displayState,
                                     const Resolution &scaledResolution,
                                     const std::shared_ptr<VideoMetadata> &videoMetadata,
                                     SDL_Rect &rect) {
  int srcW = videoMetadata->width;
  int srcH = videoMetadata->height;
  float panScaling = (videoMetadata->width * displayState.zoom.multiplier()) /
    scaledResolution.w;
  if(scaledResolution.w <= width) {
    rect.w = srcW;
  } else {
    rect.w = srcW * width / scaledResolution.w;
  }
  if(scaledResolution.h <= height) {
    rect.h = srcH;
  } else {
    rect.h = srcH * height / scaledResolution.h;
  }
  rect.x = fitToRange((srcW - rect.w) / 2 + displayState.panX * panScaling, 0,
                      srcW - rect.w);
  rect.y = fitToRange((srcH - rect.h) / 2 + displayState.panY * panScaling, 0,
                      srcH - rect.h);
}

void vivictpp::ui::ScreenOutput::setDefaultSourceRectangles(const DisplayState &displayState) {
  vivictpp::ui::setRectangle(sourceRectLeft, 0, 0, leftVideoMetadata->width, leftVideoMetadata->height);
  if (!displayState.splitScreenDisabled) {
    vivictpp::ui::setRectangle(sourceRectRight, 0, 0, rightVideoMetadata->width, rightVideoMetadata->height);
  }
}

void vivictpp::ui::ScreenOutput::updateRectangles(const DisplayState &displayState) {
  float splitPercent = displayState.splitScreenDisabled ? 100 : displayState.splitPercent;

  Resolution scaledResolution = targetResolution.scale(displayState.zoom.multiplier());
  bool fitToScreen = displayState.zoom.get() == 0;
  if (width >= scaledResolution.w && height >= scaledResolution.h ) {
    destRect.w = scaledResolution.w;
    destRect.h = scaledResolution.h;
    setDefaultSourceRectangles(displayState);
  } else if(fitToScreen) {
    if (width / static_cast<double>(height) <= scaledResolution.aspectRatio()) {
      destRect.w = width;
      destRect.h = scaledResolution.h * width / scaledResolution.w;
    } else {
      destRect.h = height;
      destRect.w = scaledResolution.w * height / scaledResolution.h;
    }
    setDefaultSourceRectangles(displayState);
  } else {
    destRect.w = std::min(width, scaledResolution.w);
    destRect.h = std::min(height, scaledResolution.h);
    calcZoomedSrcRect(displayState, scaledResolution, leftVideoMetadata, sourceRectLeft);
    if (!displayState.splitScreenDisabled) {
      calcZoomedSrcRect(displayState, scaledResolution, rightVideoMetadata, sourceRectRight);
    }
  }
  destRect.x = (width - destRect.w) / 2;
  destRect.y = (height - destRect.h) / 2;

  destRectLeft.w = (int) (destRect.w * splitPercent / 100);
  destRectLeft.h = destRect.h;
  destRectLeft.x = destRect.x;
  destRectLeft.y = destRect.y;

  destRectRight.w = destRect.w - destRectLeft.w;
  destRectRight.h = destRect.h;
  destRectRight.x = destRect.x + destRectLeft.w;
  destRectRight.y = destRect.y;

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
  SDL_SetRenderDrawColor(renderer.get(), 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer.get());
  splashText.render(renderer.get());
  SDL_RenderPresent(renderer.get());
}

void vivictpp::ui::ScreenOutput::displayFrame(
    const std::array<vivictpp::libav::Frame, 2> &frames,
    const DisplayState &displayState) {
  float splitPercent = displayState.splitPercent;
  AVFrame *frame1 = frames[0].avFrame();
  AVFrame *frame2 = frames[1].avFrame();
  SDL_GetRendererOutputSize(renderer.get(), &width, &height);
  logger->trace("renderWidth={} renderHeight={}", width, height);
  updateRectangles(displayState);
  SDL_UpdateYUVTexture(
                       leftTexture.get(), nullptr,
                       frame1->data[0], frame1->linesize[0],
                       frame1->data[1], frame1->linesize[1],
                       frame1->data[2], frame1->linesize[2]);
  if (frame2 != nullptr) {
      SDL_UpdateYUVTexture(
                       rightTexture.get(), nullptr,
                       frame2->data[0], frame2->linesize[0],
                       frame2->data[1], frame2->linesize[1],
                       frame2->data[2], frame2->linesize[2]);

  }
  SDL_SetRenderDrawColor(renderer.get(), 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer.get());

  SDL_RenderSetClipRect(renderer.get(), &destRectLeft);
  SDL_RenderCopy(renderer.get(), leftTexture.get(), &sourceRectLeft, &destRect);
  if (!displayState.splitScreenDisabled) {
    SDL_RenderSetClipRect(renderer.get(), &destRectRight);
    SDL_RenderCopy(renderer.get(), rightTexture.get(), &sourceRectRight, &destRect);
  }
  SDL_RenderSetClipRect(renderer.get(), nullptr);
  if (displayState.displayTime) {
    drawTime(displayState);
  }
  if (displayState.displayMetadata) {
    leftMetadataBox.render(renderer.get());
    if (frame2 != nullptr) {
      rightMetadataBox.render(renderer.get());
    }
  }
  if (!displayState.isPlaying && displayState.displayMetadata) {
      std::string text = std::string("Frametype: ")
                         + av_get_picture_type_char(frame1->pict_type)
                         + std::string("\nFrame size: ") + std::to_string(frame1->pkt_size);
      if (!sourceConfigs[0].vmafLog.empty()) {
        int frameN = (int)((displayState.pts - leftVideoMetadata->startTime)
                           * leftVideoMetadata->frameRate);
        text += std::string("\nVmaf score: ")
                + std::to_string(sourceConfigs[0].vmafLog.getVmafValues()[frameN]);
      }
      leftFrameBox.setText(text);
      leftFrameBox.render(renderer.get());
      if (frame2 != nullptr) {
        text = std::string("Frametype: ")
                       + av_get_picture_type_char(frame2->pict_type)
               + std::string("\nFrame size: ") + std::to_string(frame2->pkt_size);
        if (!sourceConfigs[1].vmafLog.empty()) {
          int frameN = (int)((displayState.pts - rightVideoMetadata->startTime)
                         * rightVideoMetadata->frameRate);
          text += std::string("\nVmaf score: ")
                  + std::to_string(sourceConfigs[1].vmafLog.getVmafValues()[frameN]);
        }
        rightFrameBox.setText(text);
        rightFrameBox.render(renderer.get());
      }
  }
  if (displayState.leftFrameOffset != 0) {
    frameOffsetBox.setText(std::string("  ") + std::to_string(displayState.leftFrameOffset));
    int y = displayState.displayMetadata ? ( displayState.isPlaying ? 140 : 202) : 0;
    frameOffsetBox.setY(y);
    frameOffsetBox.render(renderer.get());
  }
  if (frame2 != nullptr) {
    SDL_SetRenderDrawBlendMode(renderer.get(), SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer.get(), 255, 255, 255, 50);
    int x = (int)(width * splitPercent / 100);
    SDL_RenderDrawLine(renderer.get(), x, 0, x, height);
  }
  if (displayState.displayPlot && !vmafGraph.empty()) {
    vmafGraph.render(renderer.get(), displayState.pts, leftVideoMetadata->startTime,
                     leftVideoMetadata->duration);
  }
  if (displayState.seekBarVisible) {
    int x0 = seekBarLeftMargin;
    int y0 = height - seekBarHeight - seekBarBottomMargin;
    int w = width - 2 * seekBarLeftMargin;
    int h = seekBarHeight;
    SDL_SetRenderDrawColor(renderer.get(), 125, 125, 125, displayState.seekBarOpacity);
    SDL_Rect seekBarRect {x0, y0, w, h};
    SDL_RenderFillRect(renderer.get(), &seekBarRect);
    SDL_SetRenderDrawColor(renderer.get(), 125, 255, 125, displayState.seekBarOpacity);
    seekBarRect.w = (int) (seekBarRect.w * displayState.seekBarRelativePos);
    SDL_RenderFillRect(renderer.get(), &seekBarRect);
    seekBarRect.x = seekBarLeftMargin + seekBarRect.w - 2;
    seekBarRect.y = y0 - 3;
    seekBarRect.w = 5;
    seekBarRect.h = seekBarHeight + 6;
    SDL_SetRenderDrawColor(renderer.get(), 255, 255, 255, displayState.seekBarOpacity);
    SDL_RenderFillRect(renderer.get(), &seekBarRect);
    SDL_SetRenderDrawColor(renderer.get(), 0, 0, 0, displayState.seekBarOpacity);
    SDL_RenderDrawRect(renderer.get(), &seekBarRect);
  }
  SDL_RenderPresent(renderer.get());
}

vivictpp::ui::ClickTarget vivictpp::ui::ScreenOutput::getClickTarget(int x, int y, const vivictpp::ui::DisplayState &displayState) {
  vivictpp::ui::ClickTarget seekBar("seekbar",
                      seekBarLeftMargin,
                      height - seekBarHeight - seekBarBottomMargin,
                      width - 2 * seekBarLeftMargin,
                      seekBarHeight);
  vivictpp::ui::ClickTarget plot("plot",
                   0,
                   (int) height * 0.7,
                   width,
                   height - (int) height * 0.7);
  vivictpp::ui::ClickTarget defaultTarget("default",0,0,width,height);

  if (displayState.displayPlot && plot.contains(x,y)) {
    return plot;
  }
  if (seekBar.contains(x,y)) {
    return seekBar;
  }
  return defaultTarget;
}
