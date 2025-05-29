// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIVICTPP_UI_VIDEOTEXTURES_HH_
#define VIVICTPP_UI_VIDEOTEXTURES_HH_

#include "Resolution.hh"
#include "sdl/SDLUtils.hh"
#include "ui/DisplayState.hh"

namespace vivictpp::ui {

class VideoTextures {
public:
  vivictpp::sdl::SDLTexture leftTexture;
  vivictpp::sdl::SDLTexture rightTexture;
  Resolution nativeResolution;
  AVRational nativeAspectRatio;

public:
  bool update(SDL_Renderer *renderer, const DisplayState &displayState);

private:
  bool initTextures(SDL_Renderer *renderer, const DisplayState &displayState);
  void calcNativeResolution(const DisplayState &displayState);
  int videoMetadataVersion{-1};
};
} // namespace vivictpp::ui

#endif /* VIVICTPP_UI_VIDEOTEXTURES_HH_ */
