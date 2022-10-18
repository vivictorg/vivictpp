// SPDX-FileCopyrightText: 2022 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "libav/Utils.hh"

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
