// SPDX-FileCopyrightText: 2024 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ui/ThumbnailTexture.hh"

#include "video/VideoIndexer.hh"

vivictpp::sdl::SDLTexture &vivictpp::ui::ThumbnailTexture::updateAndGetTexture(const vivictpp::time::Time &pts) {
    if (!videoIndex) {
        return texture;
    }
    const auto &thumbnail = getThumbnail(pts);
    if (thumbnail.pts != currentTime && !thumbnail.frame.empty()) {
        auto frame = thumbnail.frame;
        if (!texture || frame->width != this->width || frame->height != this->height) {
            texture = vivictpp::sdl::SDLTexture(this->renderer, frame->width, frame->height, SDL_PIXELFORMAT_YV12);
            this->width = frame->width;
            this->height = frame->height;
        }
        texture.update(frame);
        currentTime = thumbnail.pts;
    }
    return texture;
}

const vivictpp::video::Thumbnail &vivictpp::ui::ThumbnailTexture::getThumbnail(const vivictpp::time::Time &pts) {
    const auto &thumbnails = videoIndex->getThumbnails();
    for (size_t i = 1; i < thumbnails.size(); i++) {
        if (thumbnails[i].pts > pts) {
            return thumbnails[i-1];
        }
    }
    return thumbnails[thumbnails.size()-1];
}
