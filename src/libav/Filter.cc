// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "libav/Filter.hh"

#include <exception>
#include <cstdio>
#include <memory>
#include <functional>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
}

#include "spdlog/spdlog.h"
#include "libav/AVErrorUtils.hh"

int64_t getValidChannelLayout(int64_t channelLayout, int channels);

void freeFilterGraph(AVFilterGraph *graph) {
  avfilter_graph_free(&graph);
}

vivictpp::libav::Filter::Filter() :
  bufferSrcCtx(nullptr),
  bufferSinkCtx(nullptr),
  graph(nullptr),
  eof_(false) {
}

void vivictpp::libav::Filter::configureGraph(std::string definition) {
  int ret;
  AVFilterInOut *inputs = avfilter_inout_alloc();
  AVFilterInOut *outputs = avfilter_inout_alloc();

  outputs->name       = av_strdup("in");
  outputs->filter_ctx = bufferSrcCtx;
  outputs->pad_idx    = 0;
  outputs->next       = nullptr;

  inputs->name       = av_strdup("out");
  inputs->filter_ctx = bufferSinkCtx;
  inputs->pad_idx    = 0;
  inputs->next       = nullptr;

  if ((ret = avfilter_graph_parse_ptr(graph.get(), definition.c_str(),
                                      &inputs, &outputs, NULL)) < 0) {
    throw std::runtime_error(std::string("Failed to parse filter graph ") + definition);
  }

  if ((ret = avfilter_graph_config(graph.get(), NULL)) < 0) {
    throw std::runtime_error("Failed to config filter graph");
  }
  avfilter_inout_free(&inputs);
  avfilter_inout_free(&outputs);
}

void vivictpp::libav::Filter::createFilter(AVFilterContext **filt_ctx, std::string filterName, const std::string name, const char *args, void *opaque) {
  vivictpp::libav::AVResult ret = avfilter_graph_create_filter (filt_ctx, avfilter_get_by_name(filterName.c_str()),
                                                                name.c_str(), args, opaque, graph.get());
  if (!ret.success()) {
    char buffer[1024];
    snprintf(buffer, 1024, "Failed to create filter '%s' with args '%s': %s", filterName.c_str(), args, ret.getMessage().c_str());
    throw std::runtime_error(std::string(buffer));
  }
}

vivictpp::libav::Frame vivictpp::libav::Filter::filterFrame(const vivictpp::libav::Frame &inFrame) {
  if (av_buffersrc_add_frame_flags(bufferSrcCtx, inFrame.avFrame(), AV_BUFFERSRC_FLAG_KEEP_REF) < 0) {
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

vivictpp::libav::VideoFilter::VideoFilter(AVStream *videoStream, AVCodecContext *codecContext,
                         std::string definition) :
  Filter() {
  configure(videoStream, codecContext, definition);
}


void vivictpp::libav::VideoFilter::configure(AVStream *videoStream, AVCodecContext *codecContext,
                            std::string definition) {
  char args[512];
  AVRational timeBase = videoStream->time_base;
  int ret;

  graph.reset(avfilter_graph_alloc(), &freeFilterGraph);

  enum AVPixelFormat pix_fmts[] = { AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE };

  snprintf(args, sizeof(args),
           "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
           codecContext->width, codecContext->height, codecContext->pix_fmt,
           timeBase.num, timeBase.den,
           codecContext->sample_aspect_ratio.num, codecContext->sample_aspect_ratio.den);

  createFilter( &bufferSrcCtx, "buffer", "in", args, nullptr);
  createFilter( &bufferSinkCtx, "buffersink", "out", nullptr, nullptr);

  ret = av_opt_set_int_list(bufferSinkCtx, "pix_fmts", pix_fmts,
                            AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
  if (ret < 0) {
    throw std::runtime_error("cannot set output pixel format");
  }

  configureGraph(definition);
}

vivictpp::libav::AudioFilter::AudioFilter(AVStream *audioStream, AVCodecContext *codecContext,
                         std::string definition) :
  Filter() {
  configure(audioStream, codecContext, definition);
}


void vivictpp::libav::AudioFilter::configure(AVStream *audioStream, AVCodecContext *codecContext,
                            std::string definition) {
  spdlog::debug("Audiovivictpp::libav::Filter::configure  filter definition: {}", definition);

  char args[512];

  graph.reset(avfilter_graph_alloc(), &freeFilterGraph);

  enum AVSampleFormat sample_fmts[] =  { AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_NONE };

  snprintf(args, sizeof(args),
           "sample_rate=%d:sample_fmt=%s:channels=%d:time_base=%d/%d:channel_layout=0x%" PRIx64,
           codecContext->sample_rate,
           av_get_sample_fmt_name(codecContext->sample_fmt),
           codecContext->channels,
           1, codecContext->sample_rate,
           getValidChannelLayout(codecContext->channel_layout, codecContext->channels));

  spdlog::debug("AudioFilter::configure abuffer args: {}", args);

  createFilter( &bufferSrcCtx, "abuffer", "in", args, nullptr);
  createFilter( &bufferSinkCtx, "abuffersink", "out", nullptr, nullptr);
  vivictpp::libav::AVResult ret;
  if ((ret = av_opt_set_int_list(bufferSinkCtx, "sample_fmts", sample_fmts,  AV_SAMPLE_FMT_NONE, AV_OPT_SEARCH_CHILDREN)).error()) {
    throw std::runtime_error("Failed to create audio filter: " + ret.getMessage());
  }
  if ((ret = av_opt_set_int(bufferSinkCtx, "all_channel_counts", 1, AV_OPT_SEARCH_CHILDREN)).error()) {
    throw std::runtime_error("Failed to create audio filter: " + ret.getMessage());
  }

  configureGraph(definition);
}

int64_t getValidChannelLayout(int64_t channelLayout, int channels)
{
  if (channelLayout && av_get_channel_layout_nb_channels(channelLayout) == channels)
    return channelLayout;
  else if (channels == 1) {
    return 1;
  } else {
    return 0;
  }
}
