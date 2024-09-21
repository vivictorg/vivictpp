// SPDX-FileCopyrightText: 2024 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIVICTPP_THUMBNAIL_HH
#define VIVICTPP_THUMBNAIL_HH


#include <vector>
#include "time/Time.hh"
#include "libav/Frame.hh"


namespace vivictpp::video {
        class Thumbnail {
    public:
        const vivictpp::time::Time pts;
        const vivictpp::libav::Frame frame;
    public:
        Thumbnail(const vivictpp::time::Time pts, const vivictpp::libav::Frame frame):
        pts(pts),
        frame(frame) {}
        Thumbnail(const Thumbnail &other):
        pts(other.pts),
        frame(other.frame) {}
    };
}

#endif //VIVICTPP_THUMBNAIL_HH
