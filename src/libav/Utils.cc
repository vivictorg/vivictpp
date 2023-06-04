// SPDX-FileCopyrightText: 2022 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "libav/Utils.hh"
#include "fmt/core.h"
extern "C" {
#include <libavcodec/codec.h>
}
#include <algorithm>

int vivictpp::libav::getChannels(AVCodecContext *codecContext) {
#if LIBAVCODEC_VERSION_MAJOR >= 59
  return codecContext->ch_layout.nb_channels;
#else
  return codecContext->channels;
#endif
}

int vivictpp::libav::getChannels(AVFrame *frame) {
#if LIBAVCODEC_VERSION_MAJOR >= 59
  return frame->ch_layout.nb_channels;
#else
  return frame->channels;
#endif
}

std::string vivictpp::libav::getChannelLayout(AVCodecContext *codecContext) {
  const size_t bufSize = 512;
  char buf[bufSize];
#if LIBAVCODEC_VERSION_MAJOR >= 59
  av_channel_layout_describe(&(codecContext->ch_layout), buf, bufSize);
#else
  av_get_channel_layout_string(buf, bufSize, codecContext->channels, codecContext->channel_layout);
#endif
  return buf;
}

std::vector<std::string> vivictpp::libav::allVideoDecoders() {
  std::vector<std::string> result;
  const AVCodecDescriptor *desc = nullptr;
  while ((desc = avcodec_descriptor_next(desc)) != nullptr) {
    if (desc->type == AVMEDIA_TYPE_VIDEO && avcodec_find_decoder(desc->id)) {
      void *iter = nullptr;
      const AVCodec *codec;
      while ((codec = av_codec_iterate(&iter))) {
        if (codec->id == desc->id &&
            codec->type == AVMEDIA_TYPE_VIDEO &&
            av_codec_is_decoder(codec)) {
          result.push_back(fmt::format("({}) {}", desc->name, codec->name));
        }
      }
        // av_codec_iterate
        //result.push_back(avcodec_get_name(desc->id));
    }
  }
  std::sort(result.begin(), result.end());
  return result;
}
