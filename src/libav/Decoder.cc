// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "libav/Decoder.hh"
#include "libav/AVErrorUtils.hh"

#include "spdlog/spdlog.h"

vivictpp::libav::Decoder::Decoder(AVCodecParameters *codecParameters)
  : codecContext(createCodecContext(codecParameters)) {
}

void vivictpp::libav::Decoder::flush() { avcodec_flush_buffers(this->codecContext.get()); }

std::vector<vivictpp::libav::Frame> vivictpp::libav::Decoder::handlePacket(Packet packet) {
  vivictpp::libav::AVResult ret = avcodec_send_packet(this->codecContext.get(),
                                                      packet.avPacket());
  if (ret.error()) {
    throw std::runtime_error(std::string("Send packet failed: ") + ret.getMessage());
  }
  std::vector<Frame> result;
  while ((ret = avcodec_receive_frame(this->codecContext.get(), nextFrame.avFrame())).success()) {
    result.push_back(nextFrame);
    nextFrame.reset();
  }
  if (ret.error() && !ret.eagain()) {
    throw std::runtime_error(std::string("Receive frame failed: ") + ret.getMessage());
  }
  return result;
}


std::shared_ptr<AVCodecContext> vivictpp::libav::createCodecContext(AVCodecParameters *codecParameters) {
  AVCodecID codecId = codecParameters->codec_id;
  AVCodec *codec = avcodec_find_decoder(codecId);
  if (codec == nullptr) {
    throw std::runtime_error(std::string("No codec found for codec ID: ") + std::string(avcodec_get_name(codecId)));
  }
  AVCodecContext *codecContext = avcodec_alloc_context3(codec);
  vivictpp::libav::AVResult ret = avcodec_parameters_to_context(codecContext,
                                            codecParameters);
  if (ret.error()) {
    throw std::runtime_error(std::string("Failed to copy codec parameters to context: ") +
                             ret.getMessage());
  }
  // TODO: avcodec_open2 is not thread safe so should probably be guared in some way
  if (avcodec_open2(codecContext, codec, nullptr) < 0) {
    throw std::runtime_error("Failed to open codec");
  }
  if (codecContext->codec_type == AVMEDIA_TYPE_AUDIO) {
    char buf[512];
    av_get_channel_layout_string(buf, 512, codecContext->channels, codecContext->channel_layout);
    spdlog::debug("Audio Codec Context: sample_rate={}, channels={}, channel_layout={}, format={}",
                 codecContext->sample_rate, codecContext->channels,
                 std::string(buf),
                 std::string(av_get_sample_fmt_name(codecContext->sample_fmt)));
  }
  return std::shared_ptr<AVCodecContext>(codecContext, &destroyCodecContext);
}

void vivictpp::libav::destroyCodecContext(AVCodecContext *codecContext) {
    avcodec_free_context(&codecContext);
}
