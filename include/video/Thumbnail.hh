// SPDX-FileCopyrightText: 2024 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIVICTPP_THUMBNAIL_HH
#define VIVICTPP_THUMBNAIL_HH

#include "libav/Frame.hh"
#include "time/Time.hh"
#include <vector>

namespace vivictpp::video {
class Thumbnail {
public:
  const vivictpp::time::Time pts;
  const vivictpp::libav::Frame frame;

public:
  Thumbnail(const vivictpp::time::Time pts, const vivictpp::libav::Frame frame)
      : pts(pts), frame(frame) {}
  Thumbnail(const Thumbnail &other) : pts(other.pts), frame(other.frame) {}
};
}; // namespace vivictpp::video

#endif // VIVICTPP_THUMBNAIL_HH
