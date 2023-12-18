// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ui/VideoTextures.hh"
#include "ui/SDLVideoTextures.hh"

SDL_PixelFormatEnum getTexturePixelFormat(const vivictpp::libav::Frame &frame) {
    if (!frame.empty() && (AVPixelFormat) frame.avFrame()->format == AV_PIX_FMT_NV12) {
        return SDL_PIXELFORMAT_NV12;
    }
    return SDL_PIXELFORMAT_YV12;
}

bool vivictpp::ui::SDLVideoTextures::initTextures(SDL_Renderer *renderer, const DisplayState &displayState) {
    if (videoMetadataVersion == displayState.videoMetadataVersion)
        return false;
    videoTextures.nativeResolution = getNativeResolution(displayState);
    leftTexture = vivictpp::sdl::SDLTexture(renderer,
                                            displayState.leftVideoMetadata.filteredResolution.w,
                                            displayState.leftVideoMetadata.filteredResolution.h,
                                            getTexturePixelFormat(displayState.leftFrame));
    videoTextures.leftTexture = leftTexture.get();
    if (!displayState.rightVideoMetadata.empty()) {
        rightTexture = vivictpp::sdl::SDLTexture(renderer,
                                                 displayState.rightVideoMetadata.filteredResolution.w,
                                                 displayState.rightVideoMetadata.filteredResolution.h,
                                                 getTexturePixelFormat(displayState.rightFrame));
        videoTextures.rightTexture = rightTexture.get();
    }
    videoMetadataVersion = displayState.videoMetadataVersion;
    return true;
}

bool vivictpp::ui::SDLVideoTextures::update(SDL_Renderer *renderer, const DisplayState &displayState) {
    bool textureSizeChanged = initTextures(renderer, displayState);
    leftTexture.update(displayState.leftFrame);
    if (!displayState.rightFrame.empty()) {
        rightTexture.update(displayState.rightFrame);
    }
    return textureSizeChanged;
}
