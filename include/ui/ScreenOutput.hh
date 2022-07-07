// SPDX-FileCopyrightText: 2020 Sveriges Television AB
// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef SCREENOUTPUT_HH_
#define SCREENOUTPUT_HH_

extern "C" {
#include <SDL.h>
#include <SDL_thread.h>
#include <SDL_ttf.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include "ui/DisplayState.hh"
#include "ui/TextBox.hh"
#include "ui/VmafGraph.hh"
#include "ui/Container.hh"
#include "vmaf/VmafLog.hh"
#include "VideoMetadata.hh"
#include "sdl/SDLUtils.hh"
#include "libav/Frame.hh"
#include "logging/Logging.hh"
#include "SourceConfig.hh"

#include <string>
#include <memory>
#include <functional>

// https://stackoverflow.com/questions/17579286/sdl2-0-alternative-for-sdl-overlay

namespace vivictpp {
namespace ui {

class ClickTarget {
public:
  ClickTarget(std::string name, int x, int y, int w, int h):
    name(name),
    x(x),
    y(y),
    w(w),
    h(h) { };
  const std::string name;
  const int x;
  const int y;
  const int w;
  const int h;
  bool contains(int x, int y) {
    return x >= this->x && x < this->x + this->w && y >= this->y && y < this->y + this->h;
  }
  std::string toString() {
    std::ostringstream oss;
    oss << "vivictpp::ui::ClickTarget [name=" << name
        << ",x=" << x << ",y=" << y << ",w=" << w << ",h=" << h << "]";
    return oss.str();
  }
};

class ScreenOutput {
public:
  ScreenOutput(std::vector<SourceConfig> sourceConfigs);
  ScreenOutput(const ScreenOutput&) = delete;
  ScreenOutput& operator=(const ScreenOutput&) = delete;
  virtual ~ScreenOutput();
  void displayFrame(const std::array<vivictpp::libav::Frame, 2> &frames,
                    const vivictpp::ui::DisplayState &displayState);
  int getWidth() { return width; }
  int getHeight() { return height; }
  void setFullscreen(bool fullscreen);
  void setCursorHand();
  void setCursorDefault();
  void setLeftMetadata(const VideoMetadata &metadata);
  void setRightMetadata(const VideoMetadata &metadata);
  ClickTarget getClickTarget(int x, int y, const vivictpp::ui::DisplayState &displayState);

private:
  std::shared_ptr<VideoMetadata> leftVideoMetadata;
  std::shared_ptr<VideoMetadata> rightVideoMetadata;
  std::vector<SourceConfig> sourceConfigs;
  Resolution targetResolution;

  int width;
  int height;

  vivictpp::sdl::SDLInitializer sdlInitializer;
  std::unique_ptr<SDL_Window, std::function<void(SDL_Window *)>> screen;
  std::unique_ptr<SDL_Renderer, std::function<void(SDL_Renderer *)>> renderer;

  std::unique_ptr<SDL_Texture, std::function<void(SDL_Texture *)>> leftTexture;
  std::unique_ptr<SDL_Texture, std::function<void(SDL_Texture *)>> rightTexture;
  std::unique_ptr<SDL_Cursor, std::function<void(SDL_Cursor *)>> handCursor;
  SDL_Cursor *defaultCursor;

  SDL_Rect sourceRectLeft, sourceRectRight, zoomedView, destRectLeft, destRectRight, destRect;
  FixedPositionContainer timeTextBox;
//  TextBox timeTextBox;
  bool wasMaximized;
  FixedPositionContainer leftMetaDisplay;
  FixedPositionContainer rightMetaDisplay;
  /*
  TextBox leftMetadataBox;
  TextBox rightMetadataBox;
  TextBox leftFrameBox;
  TextBox rightFrameBox;
  TextBox frameOffsetBox;
  */
  //TextBox splashText;
  FixedPositionContainer splashText;
  VmafGraph vmafGraph;
  vivictpp::logging::Logger logger;

 private:
  Resolution getTargetResolution();
  void renderSplash();
  void setSize();
  void calcZoomedSrcRect(const vivictpp::ui::DisplayState &displayState,
                         const Resolution &scaledResolution,
                         const std::shared_ptr<VideoMetadata> &videoMetadata,
                         SDL_Rect &rect);
  void setDefaultSourceRectangles(const DisplayState &displayState);
  void updateRectangles(const DisplayState &displayState);
  void initText();
  void drawTime(const vivictpp::ui::DisplayState &displayState);
  Uint8 *offsetPlaneRight(const AVFrame *frame, const int plane,
                          const vivictpp::ui::DisplayState &displayState);
  Uint8 *offsetPlaneLeft(const AVFrame *frame, const int plane,
                         const vivictpp::ui::DisplayState &displayState);
};

void setRectangle(SDL_Rect &rect, int x, int y, int w, int h);

void debugRectangle(std::string msg, const SDL_Rect &rect);

}  // ui
}  // vivictpp

#endif // SCREENOUTPUT_HH_
