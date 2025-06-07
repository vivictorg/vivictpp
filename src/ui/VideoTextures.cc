// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ui/VideoTextures.hh"

SDL_PixelFormat getTexturePixelFormat(const vivictpp::libav::Frame &frame) {
  if (!frame.empty()) {
    if ((AVPixelFormat)frame.avFrame()->format == AV_PIX_FMT_NV12) {
      return SDL_PIXELFORMAT_NV12;
    } else if ((AVPixelFormat)frame.avFrame()->format == AV_PIX_FMT_P010) {
      return SDL_PIXELFORMAT_P010;
      //return SDL_PIXELFORMAT_NV12;
    }
  }
  return SDL_PIXELFORMAT_YV12;
}

bool isAspectRatio169(const Resolution &resolution) {
  return resolution.w * 9 == resolution.h * 16;
}

// Native resolution is calculated as the resolution that can fit both videos
// after the video with narrower aspect ratio has been padded to wider AR
void vivictpp::ui::VideoTextures::calcNativeResolution(const vivictpp::ui::DisplayState &displayState) {
  Resolution leftDisplayResolution =
      displayState.leftVideoMetadata.displayResolution;
  if (displayState.rightVideoMetadata.empty()) {
    nativeResolution = leftDisplayResolution;
    return;
  }

  //AVRational displayAspectRatio{leftDisplayResolution.w, leftDisplayResolution.h};

  Resolution rightDisplayResolution =
      displayState.rightVideoMetadata.displayResolution;

  // prefer 16:9 AR
  if (isAspectRatio169(rightDisplayResolution) ||
      isAspectRatio169(leftDisplayResolution)) {
    nativeAspectRatio = {16, 9};
  } else if (leftDisplayResolution.w * rightDisplayResolution.h > 
      rightDisplayResolution.w * leftDisplayResolution.h) {
        // if none of the videos is 16:9, prefer wider aspect ratio
    nativeAspectRatio = {leftDisplayResolution.w, leftDisplayResolution.h};
  } else {
    nativeAspectRatio = {rightDisplayResolution.w, rightDisplayResolution.h};
  }

  // Now find the minimum resolution that can fit both videos after they
  // are padded to the display aspect ratio

  Resolution leftPadded = leftDisplayResolution.padToAspectRatio(nativeAspectRatio);
  Resolution rightPadded = rightDisplayResolution.padToAspectRatio(nativeAspectRatio);

  if (leftPadded.w >= rightPadded.w) {
    nativeResolution = leftPadded;
  } else {
    nativeResolution = rightPadded;
  }
}

bool vivictpp::ui::VideoTextures::initTextures(
    SDL_Renderer *renderer, const DisplayState &displayState) {
  if (videoMetadataVersion == displayState.videoMetadataVersion)
    return false;
  calcNativeResolution(displayState);
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
