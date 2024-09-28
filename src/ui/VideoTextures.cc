// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ui/VideoTextures.hh"

SDL_PixelFormatEnum getTexturePixelFormat(const vivictpp::libav::Frame &frame) {
  if (!frame.empty() &&
      (AVPixelFormat)frame.avFrame()->format == AV_PIX_FMT_NV12) {
    return SDL_PIXELFORMAT_NV12;
  }
  return SDL_PIXELFORMAT_YV12;
}

bool vivictpp::ui::VideoTextures::initTextures(
    SDL_Renderer *renderer, const DisplayState &displayState) {
  if (videoMetadataVersion == displayState.videoMetadataVersion)
    return false;
  Resolution leftDisplayResolution =
      displayState.leftVideoMetadata.filteredResolution.toDisplayResolution(
          displayState.leftVideoMetadata.filteredSampleAspectRatio);
  if (!displayState.rightVideoMetadata.empty()) {
    Resolution rightDisplayResolution =
        displayState.rightVideoMetadata.filteredResolution.toDisplayResolution(
            displayState.rightVideoMetadata.filteredSampleAspectRatio);
    nativeResolution = leftDisplayResolution.w > rightDisplayResolution.w
                           ? leftDisplayResolution
                           : rightDisplayResolution;
  } else {
    nativeResolution = leftDisplayResolution;
  }
  leftTexture = vivictpp::sdl::SDLTexture(
      renderer, displayState.leftVideoMetadata.filteredResolution.w,
      displayState.leftVideoMetadata.filteredResolution.h,
      getTexturePixelFormat(displayState.leftFrame));
  if (!displayState.rightVideoMetadata.empty()) {
    rightTexture = vivictpp::sdl::SDLTexture(
        renderer, displayState.rightVideoMetadata.filteredResolution.w,
        displayState.rightVideoMetadata.filteredResolution.h,
        getTexturePixelFormat(displayState.rightFrame));
  }
  videoMetadataVersion = displayState.videoMetadataVersion;
  return true;
}

bool vivictpp::ui::VideoTextures::update(SDL_Renderer *renderer,
                                         const DisplayState &displayState) {
  bool textureSizeChanged = initTextures(renderer, displayState);
  leftTexture.update(displayState.leftFrame);
  if (!displayState.rightFrame.empty()) {
    rightTexture.update(displayState.rightFrame);
  }
  return textureSizeChanged;
}
