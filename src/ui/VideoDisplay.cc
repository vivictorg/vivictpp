#include "ui/VideoDisplay.hh"
#include "Resolution.hh"
#include "SDL_pixels.h"

int inline fitToRange(int value, int min, int max) {
  return std::max(min, std::min(max, value));
}

vivictpp::ui::VideoDisplay::VideoDisplay():
   logger(vivictpp::logging::getOrCreateLogger("VideoDisplay")){
}

void vivictpp::ui::VideoDisplay::update(const DisplayState &displayState) {
  (void) displayState;

}

void vivictpp::ui::VideoDisplay::render(const DisplayState &displayState, SDL_Renderer *renderer, int x, int y) {
  (void) x;
  (void) y;
  if (videoTextures.update(renderer, displayState)) {
      videoScaler.defaultResolution = videoTextures.nativeResolution;
  }
  float splitPercent = displayState.splitPercent;

  SDL_GetRendererOutputSize(renderer, &box.w, &box.h);
  Resolution displaySize(box.w, box.h);
  videoScaler.update(displayState, displaySize);

  SDL_RenderSetClipRect(renderer, &videoScaler.destRectLeft);
  SDL_RenderCopy(renderer,   videoTextures.leftTexture.get(), &videoScaler.sourceRectLeft, &videoScaler.destRect);
  if (!displayState.splitScreenDisabled) {
    SDL_RenderSetClipRect(renderer, &videoScaler.destRectRight);
    SDL_RenderCopy(renderer,   videoTextures.rightTexture.get(), &videoScaler.sourceRectRight, &videoScaler.destRect);
  }
  SDL_RenderSetClipRect(renderer, nullptr);

  if (!displayState.splitScreenDisabled) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 50);
    int x = (int)(box.w * splitPercent / 100);
    SDL_RenderDrawLine(renderer, x, 0, x, box.h);
  }

}
