// SPDX-FileCopyrightText: 2022 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef UI_VIDEODISPLAY_HH
#define UI_VIDEODISPLAY_HH

extern "C" {
#include <SDL.h>
#include <SDL_thread.h>
}

#include "ui/Ui.hh"
#include "sdl/SDLUtils.hh"
#include "logging/Logging.hh"

namespace vivictpp::ui {

class VideoDisplay : public Component {
public:
  VideoDisplay();
  ~VideoDisplay() = default;
  void render(const DisplayState &displayState, SDL_Renderer *renderer, int x, int y) override;
  void update(const DisplayState &displayState);
  const Box& getBox() const override {
    return box;
  }
private:
  void initTextures(SDL_Renderer *renderer, const DisplayState &displayState);
  void calcZoomedSrcRect(const vivictpp::ui::DisplayState &displayState,
                         const Resolution &scaledResolution,
                         const VideoMetadata &videoMetadata,
                         SDL_Rect &rect);
  void setDefaultSourceRectangles(const DisplayState &displayState);
  void updateRectangles(const DisplayState &displayState, SDL_Renderer *renderer);
private:
  vivictpp::sdl::SDLTexture leftTexture;
  vivictpp::sdl::SDLTexture rightTexture;
  SDL_Rect sourceRectLeft, sourceRectRight, zoomedView, destRectLeft, destRectRight, destRect;
  Box box;
  vivictpp::logging::Logger logger;
  Resolution targetResolution;
  int videoMetadataVersion{-1};
};


} // namespace vivictpp::ui

#endif // UI_VIDEODISPLAY_HH
