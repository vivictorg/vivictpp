// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "libav/Filter.hh"

#include <cstdio>
#include <exception>
#include <functional>
#include <libavutil/pixfmt.h>
#include <locale>
#include <memory>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
}

#include "libav/AVErrorUtils.hh"
#include "libav/HwAccelUtils.hh"
#include "libav/Utils.hh"
#include "spdlog/spdlog.h"

int64_t getValidChannelLayout(int64_t channelLayout, int channels);

void freeFilterGraph(AVFilterGraph *graph) { avfilter_graph_free(&graph); }

vivictpp::libav::Filter::Filter(std::string definition)
    : eof_(false), bufferSrcCtx(nullptr), bufferSinkCtx(nullptr),
      graph(nullptr), definition(definition) {}

void vivictpp::libav::Filter::configureGraph(std::string definition) {
  spdlog::info("Configuration filter graph: {}", definition);
  int ret;
  AVFilterInOut *inputs = avfilter_inout_alloc();
  AVFilterInOut *outputs = avfilter_inout_alloc();

  outputs->name = av_strdup("in");
  outputs->filter_ctx = bufferSrcCtx;
  outputs->pad_idx = 0;
  outputs->next = nullptr;

  inputs->name = av_strdup("out");
  inputs->filter_ctx = bufferSinkCtx;
  inputs->pad_idx = 0;
  inputs->next = nullptr;

  if ((ret = avfilter_graph_parse_ptr(graph.get(), definition.c_str(), &inputs,
                                      &outputs, NULL)) < 0) {
    throw std::runtime_error(std::string("Failed to parse filter graph ") +
                             definition);
  }

  if ((ret = avfilter_graph_config(graph.get(), NULL)) < 0) {
    throw std::runtime_error("Failed to config filter graph");
  }
  avfilter_inout_free(&inputs);
  avfilter_inout_free(&outputs);
}

void vivictpp::libav::Filter::createFilter(AVFilterContext **filt_ctx,
                                           std::string filterName,
                                           const std::string name,
                                           const char *args, void *opaque) {
  vivictpp::libav::AVResult ret = avfilter_graph_create_filter(
      filt_ctx, avfilter_get_by_name(filterName.c_str()), name.c_str(), args,
      opaque, graph.get());
  if (!ret.success()) {
    char buffer[1024];
    snprintf(buffer, 1024, "Failed to create filter '%s' with args '%s': %s",
             filterName.c_str(), args, ret.getMessage().c_str());
    throw std::runtime_error(std::string(buffer));
  }
}

vivictpp::libav::Frame
vivictpp::libav::Filter::filterFrame(const vivictpp::libav::Frame &inFrame) {
  if (av_buffersrc_add_frame_flags(bufferSrcCtx, inFrame.avFrame(),
                                   AV_BUFFERSRC_FLAG_KEEP_REF) < 0) {
    throw std::runtime_error("Error feeding filtergraph");
  }
  int ret = av_buffersink_get_frame(bufferSinkCtx, nextFrame.avFrame());
  if (ret == AVERROR(EAGAIN))
    return Frame::emptyFrame();
  if (ret == AVERROR_EOF) {
    eof_ = true;
    return Frame::emptyFrame();
  }
  if (ret < 0) {
    throw std::runtime_error("Error getting frame from filtergraph");
  }
  Frame frame = nextFrame;
  nextFrame.reset();
  return frame;
}

vivictpp::libav::VideoFilter::VideoFilter(AVStream *videoStream,
                                          AVCodecContext *codecContext,
                                          std::string definition)
    : Filter(definition),
      formatParameters({videoStream->time_base, codecContext->width,
                        codecContext->height, codecContext->pix_fmt,
                        codecContext->pix_fmt,
                        codecContext->sample_aspect_ratio}) {
  configure();
}

vivictpp::libav::Frame vivictpp::libav::VideoFilter::filterFrame(
    const vivictpp::libav::Frame &inFrame) {
  if (inFrame.avFrame()->format != formatParameters.pixelFormat ||
      reconfigure) {
    reconfigure = false;
    AVPixelFormat newFormat = (AVPixelFormat)inFrame.avFrame()->format;
    spdlog::info("Reconfiguring filter, pixel format changed from {} to {}",
                 av_get_pix_fmt_name(formatParameters.pixelFormat),
                 av_get_pix_fmt_name(newFormat));
    formatParameters.pixelFormat = newFormat;
    formatParameters.hwFramesContext = inFrame.avFrame()->hw_frames_ctx;
    configure();
  }
  return Filter::filterFrame(inFrame);
}

void vivictpp::libav::VideoFilter::configure() {
  char args[1024];
  int ret;

  graph.reset(avfilter_graph_alloc(), &freeFilterGraph);

  AVPixelFormat hwDownloadFormat = AV_PIX_FMT_NONE;
  AVPixelFormat outputFormat = AV_PIX_FMT_YUV420P;
  std::string hwFilter = "";

  std::string filterStr;

  if (isHwAccelFormat(formatParameters.pixelFormat)) {
    if (formatParameters.pixelFormat == AV_PIX_FMT_CUDA &&
        avfilter_get_by_name("scale_cuda")) {
      hwFilter = "scale_cuda=format=p010";
      hwDownloadFormat = AV_PIX_FMT_P010;
    } else if (formatParameters.pixelFormat == AV_PIX_FMT_VAAPI &&
               avfilter_get_by_name("scale_vaapi")) {
      hwFilter = "scale_vaapi=format=nv12";
      hwDownloadFormat = AV_PIX_FMT_NV12;
    }

    if (hwDownloadFormat == AV_PIX_FMT_NONE) {
      if (formatParameters.pixelFormat == AV_PIX_FMT_VAAPI) {
        switch (formatParameters.sourcePixelFormat) {
        case AV_PIX_FMT_YUV420P10LE:
        case AV_PIX_FMT_YUV420P10BE:
          hwDownloadFormat = AV_PIX_FMT_P010;
          break;
        default:
          hwDownloadFormat = AV_PIX_FMT_NV12;
        }
      } else {
        hwDownloadFormat =
            selectSwPixelFormat(formatParameters.hwFramesContext);
      }
    }
    if (hwDownloadFormat == AV_PIX_FMT_NV12 ||
        hwDownloadFormat == AV_PIX_FMT_P010) {
      outputFormat = hwDownloadFormat;
    }
  }

  enum AVPixelFormat pix_fmts[] = {AV_PIX_FMT_NV12, AV_PIX_FMT_NONE};
  pix_fmts[0] = outputFormat;

  snprintf(args, sizeof(args),
           "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
           formatParameters.width, formatParameters.height,
           formatParameters.pixelFormat, formatParameters.timeBase.num,
           formatParameters.timeBase.den,
           formatParameters.sampleAspectRatio.num,
           formatParameters.sampleAspectRatio.den);

  createFilter(&bufferSrcCtx, "buffer", "in", args, nullptr);
  createFilter(&bufferSinkCtx, "buffersink", "out", nullptr, nullptr);
  if (formatParameters.hwFramesContext) {
    AVBufferSrcParameters *bufferSrcParameters =
        av_buffersrc_parameters_alloc();
    bufferSrcParameters->hw_frames_ctx = formatParameters.hwFramesContext;
    vivictpp::libav::AVResult res =
        av_buffersrc_parameters_set(bufferSrcCtx, bufferSrcParameters);
    if (res.error()) {
      throw new std::runtime_error("Failed to set buffersrc parameters " +
                                   res.getMessage());
    }
  }
  ret = av_opt_set_int_list(bufferSinkCtx, "pix_fmts", pix_fmts,
                            AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);          
  if (ret < 0) {
    throw std::runtime_error("cannot set output pixel format");
  }

  if (!hwFilter.empty()) {
    filterStr = hwFilter + ",";
  }
  if (hwDownloadFormat != AV_PIX_FMT_NONE) {
    filterStr += std::string("hwdownload,format=") +
                 av_get_pix_fmt_name(hwDownloadFormat) + ",";
  }

  filterStr += std::string("scale=sws_dither=none:dst_format=") + av_get_pix_fmt_name(outputFormat);

  if (!definition.empty()) {
    filterStr += std::string(",") + definition;
  }

  configureGraph(filterStr);
}

FilteredVideoMetadata vivictpp::libav::VideoFilter::getFilteredVideoMetadata() {
  int w = av_buffersink_get_w(bufferSinkCtx);
  int h = av_buffersink_get_h(bufferSinkCtx);
  double frameRate = av_q2d(av_buffersink_get_frame_rate(bufferSinkCtx));
  AVRational sampleAspectRatio =
      av_buffersink_get_sample_aspect_ratio(bufferSinkCtx);
  return FilteredVideoMetadata(definition, Resolution(w, h), sampleAspectRatio,
                               frameRate);
}

vivictpp::libav::AudioFilter::AudioFilter(AVCodecContext *codecContext,
                                          std::string definition)
    : Filter(definition) {
  configure(codecContext, definition);
}

void vivictpp::libav::AudioFilter::configure(AVCodecContext *codecContext,
                                             std::string definition) {
  spdlog::debug(
      "Audiovivictpp::libav::Filter::configure  filter definition: {}",
      definition);

  char args[512];

  graph.reset(avfilter_graph_alloc(), &freeFilterGraph);

  enum AVSampleFormat sample_fmts[] = {AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_NONE};

  int channels = getChannels(codecContext);
  std::string channelLayout = getChannelLayout(codecContext);

  snprintf(args, sizeof(args),
           "sample_rate=%d:sample_fmt=%s:channels=%d:time_base=%d/"
           "%d:channel_layout=%s",
           codecContext->sample_rate,
           av_get_sample_fmt_name(codecContext->sample_fmt), channels, 1,
           codecContext->sample_rate, channelLayout.c_str());

  spdlog::debug("AudioFilter::configure abuffer args: {}", args);

  createFilter(&bufferSrcCtx, "abuffer", "in", args, nullptr);
  createFilter(&bufferSinkCtx, "abuffersink", "out", nullptr, nullptr);
  vivictpp::libav::AVResult ret;
  if ((ret = av_opt_set_int_list(bufferSinkCtx, "sample_fmts", sample_fmts,
                                 AV_SAMPLE_FMT_NONE, AV_OPT_SEARCH_CHILDREN))
          .error()) {
    throw std::runtime_error("Failed to create audio filter: " +
                             ret.getMessage());
  }
  if ((ret = av_opt_set_int(bufferSinkCtx, "all_channel_counts", 1,
                            AV_OPT_SEARCH_CHILDREN))
          .error()) {
    throw std::runtime_error("Failed to create audio filter: " +
                             ret.getMessage());
  }

  configureGraph(definition);
}
