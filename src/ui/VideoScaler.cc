// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ui/VideoScaler.hh"
#include "Resolution.hh"

int inline fitToRange(int value, int min, int max) {
  return std::max(min, std::min(max, value));
}

void vivictpp::ui::VideoScaler::update(const DisplayState &displayState, Resolution displaySize) {
  int width = displaySize.w, height = displaySize.h;

  float splitPercent = displayState.splitScreenDisabled ? 100 : displayState.splitPercent;

  Resolution renderResolution = displayState.fitToScreen ? defaultResolution.scaleKeepingAspectRatio(width, height) :
                               defaultResolution;
  Resolution scaledResolution = renderResolution.scale(displayState.zoom.multiplier());


  bool keepAspectRatio = displayState.zoom.get() == 0;

  if (width >= scaledResolution.w && height >= scaledResolution.h ) {
    destRect.w = scaledResolution.w;
    destRect.h = scaledResolution.h;
    setDefaultSourceRectangles(displayState);
  } else if(keepAspectRatio) {
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
    calcZoomedSrcRect(displayState, displaySize, scaledResolution, displayState.leftVideoMetadata, sourceRectLeft);
    if (!displayState.splitScreenDisabled) {
      calcZoomedSrcRect(displayState, displaySize, scaledResolution, displayState.rightVideoMetadata, sourceRectRight);
    }
  }
  destRect.x = (width - destRect.w) / 2;
  destRect.y = (height - destRect.h) / 2;

  destRectLeft = {destRect.x, destRect.y, (int) (destRect.w * splitPercent / 100), destRect.h};
  destRectRight = {destRect.x + destRectLeft.w, destRect.y, destRect.w - destRectLeft.w, destRect.h};
}

void vivictpp::ui::VideoScaler::setDefaultSourceRectangles(const DisplayState &displayState) {
  sourceRectLeft = {0, 0, displayState.leftVideoMetadata.filteredResolution.w,
    displayState.leftVideoMetadata.filteredResolution.h};
  if (!displayState.splitScreenDisabled) {
    sourceRectRight = {0, 0, displayState.rightVideoMetadata.filteredResolution.w,
      displayState.rightVideoMetadata.filteredResolution.h};
  }
}

void vivictpp::ui::VideoScaler::calcZoomedSrcRect(const DisplayState &displayState,
                                                  const Resolution &displaySize,
                                                  const Resolution &scaledResolution,
                                                  const VideoMetadata &videoMetadata,
                                                  Rectangle &rect) {
  int srcW = videoMetadata.filteredResolution.w;
  int srcH = videoMetadata.filteredResolution.h;
  float panScaling = (videoMetadata.filteredResolution.w * displayState.zoom.multiplier()) /
    scaledResolution.w;
  if(scaledResolution.w <= displaySize.w) {
    rect.w = srcW;
  } else {
    rect.w = srcW * displaySize.w / scaledResolution.w;
  }
  if(scaledResolution.h <= displaySize.h) {
    rect.h = srcH;
  } else {
    rect.h = srcH * displaySize.h / scaledResolution.h;
  }
  rect.x = fitToRange((srcW - rect.w) / 2 + displayState.panX * panScaling, 0,
                      srcW - rect.w);
  rect.y = fitToRange((srcH - rect.h) / 2 + displayState.panY * panScaling, 0,
                      srcH - rect.h);
}
