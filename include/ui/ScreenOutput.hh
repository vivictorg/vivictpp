// SPDX-FileCopyrightText: 2020 Sveriges Television AB
// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef UI_SCREENOUTPUT_HH
#define UI_SCREENOUTPUT_HH

extern "C" {
#include <SDL.h>
#include <SDL_thread.h>
#include <SDL_ttf.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include "SourceConfig.hh"
#include "VideoMetadata.hh"
#include "libav/Frame.hh"
#include "logging/Logging.hh"
#include "sdl/SDLUtils.hh"
#include "ui/Container.hh"
#include "ui/DisplayState.hh"
#include "ui/Events.hh"
#include "ui/MetadataDisplay.hh"
#include "ui/SeekBar.hh"
#include "ui/Splash.hh"
#include "ui/TextBox.hh"
#include "ui/VideoDisplay.hh"
#include "ui/VmafGraph.hh"
#include "vmaf/VmafLog.hh"

#include <string>
#include <memory>
#include <functional>

// https://stackoverflow.com/questions/17579286/sdl2-0-alternative-for-sdl-overlay

namespace vivictpp::ui {

class ScreenOutput {
public:
  ScreenOutput(std::vector<SourceConfig> sourceConfigs);
  ScreenOutput(const ScreenOutput&) = delete;
  ScreenOutput& operator=(const ScreenOutput&) = delete;
  virtual ~ScreenOutput();
  void displayFrame(const vivictpp::ui::DisplayState &displayState);
  int getWidth() { return width; }
  int getHeight() { return height; }
  void onResize();
  void setFullscreen(bool fullscreen);
  void setCursorHand();
  void setCursorPan();
  void setCursorDefault();
  void setLeftMetadata(const VideoMetadata &metadata);
  void setRightMetadata(const VideoMetadata &metadata);
  const MouseClicked getClickTarget(int x, int y);

private:
  std::vector<SourceConfig> sourceConfigs;
  Resolution targetResolution;

  int width;
  int height;

  std::unique_ptr<SDL_Window, std::function<void(SDL_Window *)>> screen;
  std::unique_ptr<SDL_Renderer, std::function<void(SDL_Renderer *)>> renderer;
  std::unique_ptr<SDL_Cursor, std::function<void(SDL_Cursor *)>> handCursor;
  std::unique_ptr<SDL_Cursor, std::function<void(SDL_Cursor *)>> panCursor;
  SDL_Cursor *defaultCursor;

  VideoDisplay videoDisplay;
  FixedPositionContainer timeTextBox;
  bool wasMaximized;
  MetadataDisplay leftMetaDisplay;
  MetadataDisplay rightMetaDisplay;
  Splash splashText;
  VmafGraph vmafGraph;
  SeekBar seekBar;
  vivictpp::logging::Logger logger;
  int videoMetadataVersion{-1};

 private:
  Resolution getTargetResolution(const VideoMetadata &leftVideoMetadata,
                                 const VideoMetadata &rightVideoMetadata);
  void initialize(const DisplayState &displayState);
  void renderSplash();
  void setSize(Resolution targetResolution);
  void initText();
  void drawTime(const vivictpp::ui::DisplayState &displayState);
  Uint8 *offsetPlaneRight(const AVFrame *frame, const int plane,
                          const vivictpp::ui::DisplayState &displayState);
  Uint8 *offsetPlaneLeft(const AVFrame *frame, const int plane,
                         const vivictpp::ui::DisplayState &displayState);
};

void setRectangle(SDL_Rect &rect, int x, int y, int w, int h);

void debugRectangle(std::string msg, const SDL_Rect &rect);

}  // vivictpp::ui

#endif // UI_SCREENOUTPUT_HH
