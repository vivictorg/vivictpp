// SPDX-FileCopyrightText: 2024 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIVICTPP_THUMBNAILTEXTURE_HH
#define VIVICTPP_THUMBNAILTEXTURE_HH

#include "sdl/SDLUtils.hh"
#include "video/VideoIndexer.hh"

namespace vivictpp::ui {

class ThumbnailTexture {
public:
  ThumbnailTexture(SDL_Renderer *renderer)
      : renderer(renderer), width(128), height(72),
        texture(/*renderer, width, height, SDL_PIXELFORMAT_YV12*/){};
  vivictpp::sdl::SDLTexture &
  updateAndGetTexture(const vivictpp::time::Time &pts);
  void setVideoIndex(std::shared_ptr<vivictpp::video::VideoIndex> videoIndex) {
    this->videoIndex = videoIndex;
  }
  int getWidth() const { return width; }
  int getHeight() const { return height; }

private:
  const vivictpp::video::Thumbnail &
  getThumbnail(const vivictpp::time::Time &pts);

private:
  SDL_Renderer *renderer;
  vivictpp::time::Time currentTime{vivictpp::time::NO_TIME};
  std::shared_ptr<vivictpp::video::VideoIndex> videoIndex;
  int width;
  int height;
  vivictpp::sdl::SDLTexture texture;
};
} // namespace vivictpp::ui

#endif // VIVICTPP_THUMBNAILTEXTURE_HH
