// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef LIBAV_FILTER_HH
#define LIBAV_FILTER_HH

#include <atomic>
#include <memory>
#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavformat/avformat.h>
}

#include "Resolution.hh"
#include "VideoMetadata.hh"
#include "libav/Frame.hh"

namespace vivictpp {
namespace libav {

class Filter {
private:
  bool eof_;
  Frame nextFrame;

protected:
  Filter(std::string definition);
  void createFilter(AVFilterContext **filt_ctx, std::string filterName,
                    const std::string name, const char *args, void *opaque);
  void configureGraph(std::string definition);
  AVFilterContext *bufferSrcCtx;
  AVFilterContext *bufferSinkCtx;
  std::shared_ptr<AVFilterGraph> graph;
  std::string definition;
  std::atomic_bool reconfigure;

public:
  virtual ~Filter() = default;
  virtual Frame filterFrame(const Frame &frame);
  bool eof() { return eof_; };
  Resolution getFilteredResolution();
  void reconfigureOnNextFrame() { reconfigure = true; };
};

struct VideoFilterFormatParameters {
  AVRational timeBase;
  int width;
  int height;
  AVPixelFormat pixelFormat;
  AVPixelFormat sourcePixelFormat;
  AVRational sampleAspectRatio;
  AVBufferRef *hwFramesContext{nullptr};
};

class VideoFilter : public Filter {
public:
  VideoFilter(AVStream *avStream, AVCodecContext *codecContext,
              std::string definition);
  ~VideoFilter() = default;
  FilteredVideoMetadata getFilteredVideoMetadata();
  Frame filterFrame(const Frame &frame) override;

private:
  void configure();

private:
  VideoFilterFormatParameters formatParameters;
};

class AudioFilter : public Filter {
public:
  AudioFilter(AVCodecContext *codecContext, std::string definition);
  ~AudioFilter() = default;

private:
  void configure(AVCodecContext *codecContext, std::string definition);
};

} // namespace libav
} // namespace vivictpp

#endif // LIBAV_FILTER_HH
