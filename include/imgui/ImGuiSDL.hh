// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIVICTPP_IMGUI_IMGUISDL_HH_
#define VIVICTPP_IMGUI_IMGUISDL_HH_

#include "imgui/Events.hh"
#include "Settings.hh"
#include "sdl/SDLUtils.hh"
#include "ui/VideoTextures.hh"
#include "ui/ThumbnailTexture.hh"
#include "VivictPPConfig.hh"
#include "video/VideoIndexer.hh"
#include <vector>
#include <memory>
#include <filesystem>
#include <string>

namespace vivictpp::imgui {

class ImGuiSDL {
private:
  int windowWidth{1280};
  int windowHeight{720};
  sdl::SDLInitializer sdlInitializer;
  sdl::SDLWindow windowPtr;
  SDL_Window *window;
  sdl::SDLRenderer rendererPtr;
  SDL_Renderer *renderer;
  vivictpp::ui::VideoTextures videoTextures;
  bool fullscreen{false};
  std::filesystem::path iniFilename;
  std::string iniFilenameStr;
  bool scaleRenderer;
  vivictpp::ui::ThumbnailTexture thumbnailTexture;
public:
  ImGuiSDL(const Settings &settings);
  ~ImGuiSDL();
  void newFrame();
  void updateTextures(const ui::DisplayState &displayState);
  vivictpp::ui::VideoTextures &getVideoTextures() { return videoTextures; }
  void updateThumbnails(std::shared_ptr<vivictpp::video::VideoIndex> videoIndex) {
    thumbnailTexture.setVideoIndex(videoIndex);
  }
  vivictpp::ui::ThumbnailTexture &getThumbnailTexture() { return thumbnailTexture; }
  void fitWindowToTextures();
  bool isWindowClose(SDL_Event &event);
  std::vector<std::shared_ptr<Event>> handleEvents();
  void render();
  bool toggleFullscreen();
  void updateFontSettings(const Settings &settings);
};

}  // namespace vivictpp::imgui


#endif /* VIVICTPP_IMGUI_IMGUISDL_HH_ */
