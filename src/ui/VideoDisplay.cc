#include "ui/VideoDisplay.hh"

int inline fitToRange(int value, int min, int max) {
  return std::max(min, std::min(max, value));
}

vivictpp::ui::VideoDisplay::VideoDisplay():
   leftTexture(nullptr, nullptr),
   rightTexture(nullptr, nullptr),
   logger(vivictpp::logging::getOrCreateLogger("VideoDisplay")){
}

void vivictpp::ui::VideoDisplay::initTextures(SDL_Renderer *renderer, const DisplayState &displayState) {
  if (displayState.rightVideoMetadata.empty() ||
      displayState.leftVideoMetadata.filteredResolution.w > displayState.rightVideoMetadata.filteredResolution.w) {
    targetResolution = displayState.leftVideoMetadata.filteredResolution;
  } else {
    targetResolution = displayState.rightVideoMetadata.filteredResolution;
  }
  leftTexture = vivictpp::sdl::createTexture(renderer,
                                             displayState.leftVideoMetadata.filteredResolution.w,
                                             displayState.leftVideoMetadata.filteredResolution.h);
  if (!displayState.rightVideoMetadata.empty()) {
    rightTexture = vivictpp::sdl::createTexture(renderer,
                                                displayState.rightVideoMetadata.filteredResolution.w,
                                                displayState.rightVideoMetadata.filteredResolution.h);
  }
}

void vivictpp::ui::VideoDisplay::setDefaultSourceRectangles(const DisplayState &displayState) {
  sourceRectLeft = {0, 0, displayState.leftVideoMetadata.filteredResolution.w,
    displayState.leftVideoMetadata.filteredResolution.h};
  if (!displayState.splitScreenDisabled) {
    sourceRectRight = {0, 0, displayState.rightVideoMetadata.filteredResolution.w,
      displayState.rightVideoMetadata.filteredResolution.h};
  }
}

void vivictpp::ui::VideoDisplay::updateRectangles(const DisplayState &displayState, SDL_Renderer *renderer) {
  float splitPercent = displayState.splitScreenDisabled ? 100 : displayState.splitPercent;
  Resolution scaledResolution = targetResolution.scale(displayState.zoom.multiplier());
  SDL_GetRendererOutputSize(renderer, &box.w, &box.h);
  int width = box.w, height = box.h;
  logger->debug("vivictpp::ui::VideoDisplay::updateRectangles scaledResolution={}x{}", scaledResolution.w, scaledResolution.h);
  bool fitToScreen = displayState.zoom.get() == 0;
  if (width >= scaledResolution.w && height >= scaledResolution.h ) {
    destRect.w = scaledResolution.w;
    destRect.h = scaledResolution.h;
    setDefaultSourceRectangles(displayState);
  } else if(fitToScreen) {
    if (width / static_cast<double>(height) <= scaledResolution.aspectRatio()) {
      destRect.w = width;
      destRect.h = scaledResolution.h * width / scaledResolution.w;
    } else {
      destRect.h = height;
      destRect.w = scaledResolution.w * height / scaledResolution.h;
    }
    setDefaultSourceRectangles(displayState);
  } else {
    destRect.w = std::min(width, scaledResolution.w);
    destRect.h = std::min(height, scaledResolution.h);
    calcZoomedSrcRect(displayState, scaledResolution, displayState.leftVideoMetadata, sourceRectLeft);
    if (!displayState.splitScreenDisabled) {
      calcZoomedSrcRect(displayState, scaledResolution, displayState.rightVideoMetadata, sourceRectRight);
    }
  }
  destRect.x = (width - destRect.w) / 2;
  destRect.y = (height - destRect.h) / 2;

  destRectLeft = {destRect.x, destRect.y, (int) (destRect.w * splitPercent / 100), destRect.h};
  destRectRight = {destRect.x + destRectLeft.w, destRect.y, destRect.w - destRectLeft.w, destRect.h};

//  debugRectangle("left dest: ", destRectLeft);

}


void vivictpp::ui::VideoDisplay::calcZoomedSrcRect(const DisplayState &displayState,
                                     const Resolution &scaledResolution,
                                     const VideoMetadata &videoMetadata,
                                     SDL_Rect &rect) {
  int srcW = videoMetadata.filteredResolution.w;
  int srcH = videoMetadata.filteredResolution.h;
  float panScaling = (videoMetadata.filteredResolution.w * displayState.zoom.multiplier()) /
    scaledResolution.w;
  if(scaledResolution.w <= box.w) {
    rect.w = srcW;
  } else {
    rect.w = srcW * box.w / scaledResolution.w;
  }
  if(scaledResolution.h <= box.h) {
    rect.h = srcH;
  } else {
    rect.h = srcH * box.h / scaledResolution.h;
  }
  rect.x = fitToRange((srcW - rect.w) / 2 + displayState.panX * panScaling, 0,
                      srcW - rect.w);
  rect.y = fitToRange((srcH - rect.h) / 2 + displayState.panY * panScaling, 0,
                      srcH - rect.h);
}

void vivictpp::ui::VideoDisplay::update(const DisplayState &displayState) {
  (void) displayState;

}

void vivictpp::ui::VideoDisplay::render(const DisplayState &displayState, SDL_Renderer *renderer, int x, int y) {
  (void) x;
  (void) y;
  if (videoMetadataVersion != displayState.videoMetadataVersion) {
    initTextures(renderer, displayState);
    videoMetadataVersion = displayState.videoMetadataVersion;
  }
  float splitPercent = displayState.splitPercent;
  AVFrame *frame1 = displayState.leftFrame.avFrame();
  AVFrame *frame2 = displayState.rightFrame.avFrame();
//  SDL_GetRendererOutputSize(renderer.get(), &width, &height);
  //logger->trace("renderWidth={} renderHeight={}", width, height);
  updateRectangles(displayState, renderer);
  SDL_UpdateYUVTexture(
    leftTexture.get(), nullptr,
    frame1->data[0], frame1->linesize[0],
    frame1->data[1], frame1->linesize[1],
    frame1->data[2], frame1->linesize[2]);
  if (frame2 != nullptr) {
    SDL_UpdateYUVTexture(
      rightTexture.get(), nullptr,
      frame2->data[0], frame2->linesize[0],
      frame2->data[1], frame2->linesize[1],
      frame2->data[2], frame2->linesize[2]);
  }
//    SDL_GetRendererOutputSize(renderer.get(), &box.w, &box.h);
  SDL_RenderSetClipRect(renderer, &destRectLeft);
  SDL_RenderCopy(renderer, leftTexture.get(), &sourceRectLeft, &destRect);
  if (!displayState.splitScreenDisabled) {
    SDL_RenderSetClipRect(renderer, &destRectRight);
    SDL_RenderCopy(renderer, rightTexture.get(), &sourceRectRight, &destRect);
  }
  SDL_RenderSetClipRect(renderer, nullptr);

  if (!displayState.splitScreenDisabled) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 50);
    int x = (int)(box.w * splitPercent / 100);
    SDL_RenderDrawLine(renderer, x, 0, x, box.h);
  }

}
