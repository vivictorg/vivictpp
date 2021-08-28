// SPDX-FileCopyrightText: 2020 Sveriges Television AB
// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIDEOMETADATA_HH_
#define VIDEOMETADATA_HH_

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/pixdesc.h>
}

#include <iomanip>
#include <sstream>
#include <string>

#include "Resolution.hh"

class VideoMetadata {
public:
  VideoMetadata(std::string source, AVFormatContext *formatContext,
                AVStream *videoStream);
  const std::string source;
  const int streamIndex;
  const std::string pixelFormat;
  const int width;
  const int height;
  const Resolution resolution;
  const int bitrate;
  const double frameRate;
  const double startTime;
  const double duration;
  const std::string codec;

  std::string toString() const;
};

#endif // VIDEOMETADATA_HH_
