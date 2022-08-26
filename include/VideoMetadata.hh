// SPDX-FileCopyrightText: 2020 Sveriges Television AB
// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIDEOMETADATA_HH_
#define VIDEOMETADATA_HH_

#include <bits/stdint-intn.h>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/pixdesc.h>
}

#include <iomanip>
#include <sstream>
#include <string>
#include <atomic>

#include "Resolution.hh"
#include "time/Time.hh"

class FilteredVideoMetadata {
public:
    FilteredVideoMetadata(std::string filterDefinition = "",
                          Resolution resolution = Resolution(0,0),
                          double frameRate = 0);
    std::string filteredDefinition;
    Resolution resolution;
    double frameRate;
public:
    bool empty() { return filteredDefinition.empty(); }
};

class VideoMetadata {
public:
  VideoMetadata();
  VideoMetadata(std::string source,
                AVFormatContext *formatContext,
                AVStream *videoStream,
                FilteredVideoMetadata filteredVideoMetadata);
  VideoMetadata(const VideoMetadata &other) = default;
  std::string source;
  int streamIndex;
  std::string pixelFormat;
  Resolution resolution;
  Resolution filteredResolution;
  FilteredVideoMetadata filteredVideoMetadata;
  int bitrate;
  double frameRate;
  vivictpp::time::Time frameDuration;
  vivictpp::time::Time startTime;
  vivictpp::time::Time duration;
  vivictpp::time::Time endTime;
  std::string codec;

  std::string toString() const;
  bool empty() const { return _empty; }
private:
  std::string resolutionAsString() const;
  bool _empty;
};

struct FrameMetadata {
  char pictureType;
  int64_t pts;
  int64_t size;
};

#endif // VIDEOMETADATA_HH_
