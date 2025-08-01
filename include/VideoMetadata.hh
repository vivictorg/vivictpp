// SPDX-FileCopyrightText: 2020 Sveriges Television AB
// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIDEOMETADATA_HH_
#define VIDEOMETADATA_HH_

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/pixdesc.h>
}

#include <atomic>
#include <iomanip>
#include <sstream>
#include <string>

#include "Resolution.hh"
#include "time/Time.hh"

class FilteredVideoMetadata {
public:
  FilteredVideoMetadata(std::string filterDefinition = "",
                        Resolution resolution = Resolution(0, 0),
                        AVRational sampleAspectRatio = {1, 1},
                        double frameRate = 0);
  std::string filteredDefinition;
  Resolution resolution;
  AVRational sampleAspectRatio;
  double frameRate;

public:
  bool empty() const { return filteredDefinition.empty(); }
};

class VideoMetadata {
public:
  VideoMetadata();
  VideoMetadata(const std::string &source, const AVFormatContext *formatContext,
                const AVStream *videoStream,
                const FilteredVideoMetadata &filteredVideoMetadata);

  std::string source;
  std::string pixelFormat;
  int streamIndex;
  Resolution resolution;
  AVRational sampleAspectRatio;
  Resolution filteredResolution;
  AVRational filteredSampleAspectRatio;
  Resolution displayResolution;
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
  bool hasDuration() const { return duration != vivictpp::time::NO_TIME; }

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
