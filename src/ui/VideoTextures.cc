#include "ui/VideoTextures.hh"

SDL_PixelFormatEnum getTexturePixelFormat(const vivictpp::libav::Frame &frame) {
  if (!frame.empty() && (AVPixelFormat) frame.avFrame()->format == AV_PIX_FMT_NV12) {
    return SDL_PIXELFORMAT_NV12;
  }
  return SDL_PIXELFORMAT_YV12;
}

bool vivictpp::ui::VideoTextures::initTextures(SDL_Renderer *renderer, const DisplayState &displayState) {
  if (videoMetadataVersion == displayState.videoMetadataVersion)
    return false;
  if (displayState.rightVideoMetadata.empty() ||
      displayState.leftVideoMetadata.filteredResolution.w > displayState.rightVideoMetadata.filteredResolution.w) {
    nativeResolution = displayState.leftVideoMetadata.filteredResolution;
  } else {
    nativeResolution = displayState.rightVideoMetadata.filteredResolution;
  }
  leftTexture = vivictpp::sdl::SDLTexture(renderer,
                                          displayState.leftVideoMetadata.filteredResolution.w,
                                          displayState.leftVideoMetadata.filteredResolution.h,
                                          getTexturePixelFormat(displayState.leftFrame));
  if (!displayState.rightVideoMetadata.empty()) {
    rightTexture = vivictpp::sdl::SDLTexture(renderer,
                                             displayState.rightVideoMetadata.filteredResolution.w,
                                             displayState.rightVideoMetadata.filteredResolution.h,
                                             getTexturePixelFormat(displayState.rightFrame));
  }
  videoMetadataVersion = displayState.videoMetadataVersion;
  return true;
}

bool vivictpp::ui::VideoTextures::update(SDL_Renderer *renderer, const DisplayState &displayState) {
  bool textureSizeChanged = initTextures(renderer, displayState);
  leftTexture.update(displayState.leftFrame);
  if (!displayState.rightFrame.empty()) {
      rightTexture.update(displayState.rightFrame);
  }
  return textureSizeChanged;
}
