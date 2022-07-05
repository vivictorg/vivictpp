// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FILTER_H_
#define FILTER_H_

#include <string>
#include <memory>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
}

#include "libav/Frame.hh"
#include "Resolution.hh"
#include "VideoMetadata.hh"

namespace vivictpp {
namespace libav {

class Filter {
public:
  virtual ~Filter() = default;
  Frame filterFrame(const Frame &frame);
  bool eof() { return eof_; };
  Resolution getFilteredResolution();
protected:
  Filter(std::string definition);
  void createFilter(AVFilterContext **filt_ctx, std::string filterName, const std::string name, const char *args, void *opaque);
  void configureGraph(std::string definition);
  AVFilterContext *bufferSrcCtx;
  AVFilterContext *bufferSinkCtx;
  std::shared_ptr<AVFilterGraph> graph;
  std::string definition;
private:
  bool eof_;
  Frame nextFrame;
};

class VideoFilter: public Filter {
public:
  VideoFilter(AVStream *avStream, AVCodecContext *codecContext, std::string definition);
  ~VideoFilter() = default;
  FilteredVideoMetadata getFilteredVideoMetadata();
private:
    void configure(AVStream *videoStream, AVCodecContext *codecContext,
             std::string definition);
};

class AudioFilter: public Filter {
public:
  AudioFilter(AVCodecContext *codecContext, std::string definition);
  ~AudioFilter() = default;
private:
    void configure(AVCodecContext *codecContext,
             std::string definition);
};

}  // namespace libav
}  // namespace vivictpp

#endif  // FILTER_H_
