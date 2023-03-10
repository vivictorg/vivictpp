#include "ui/VideoDisplay.hh"
#include "Resolution.hh"
#include "SDL_pixels.h"

int inline fitToRange(int value, int min, int max) {
  return std::max(min, std::min(max, value));
}

SDL_PixelFormatEnum getTexturePixelFormat(const vivictpp::libav::Frame &frame) {
  if (!frame.empty() && (AVPixelFormat) frame.avFrame()->format == AV_PIX_FMT_NV12) {
    return SDL_PIXELFORMAT_NV12;
  }
  return SDL_PIXELFORMAT_YV12;
}

vivictpp::ui::VideoDisplay::VideoDisplay():
   logger(vivictpp::logging::getOrCreateLogger("VideoDisplay")){
}

void vivictpp::ui::VideoDisplay::initTextures(SDL_Renderer *renderer, const DisplayState &displayState) {
  if (displayState.rightVideoMetadata.empty() ||
      displayState.leftVideoMetadata.filteredResolution.w > displayState.rightVideoMetadata.filteredResolution.w) {
    targetResolution = displayState.leftVideoMetadata.filteredResolution;
  } else {
    targetResolution = displayState.rightVideoMetadata.filteredResolution;
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
}

void vivictpp::ui::VideoDisplay::update(const DisplayState &displayState) {
  (void) displayState;

}

void vivictpp::ui::VideoDisplay::render(const DisplayState &displayState, SDL_Renderer *renderer, int x, int y) {
  (void) x;
  (void) y;
  if (videoMetadataVersion != displayState.videoMetadataVersion) {
    initTextures(renderer, displayState);
    videoScaler.defaultResolution = targetResolution;
    videoMetadataVersion = displayState.videoMetadataVersion;
  }
  float splitPercent = displayState.splitPercent;

  SDL_GetRendererOutputSize(renderer, &box.w, &box.h);
  Resolution displaySize(box.w, box.h);
  videoScaler.update(displayState, displaySize);

  leftTexture.update(displayState.leftFrame);
  if (!displayState.rightFrame.empty()) {
    rightTexture.update(displayState.rightFrame);
  }

  SDL_RenderSetClipRect(renderer, &videoScaler.destRectLeft);
  SDL_RenderCopy(renderer, leftTexture.get(), &videoScaler.sourceRectLeft, &videoScaler.destRect);
  if (!displayState.splitScreenDisabled) {
    SDL_RenderSetClipRect(renderer, &videoScaler.destRectRight);
    SDL_RenderCopy(renderer, rightTexture.get(), &videoScaler.sourceRectRight, &videoScaler.destRect);
  }
  SDL_RenderSetClipRect(renderer, nullptr);

  if (!displayState.splitScreenDisabled) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 50);
    int x = (int)(box.w * splitPercent / 100);
    SDL_RenderDrawLine(renderer, x, 0, x, box.h);
  }

}
