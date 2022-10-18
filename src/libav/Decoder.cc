// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "libav/Decoder.hh"
#include "libav/AVErrorUtils.hh"
#include "libav/Utils.hh"

extern "C" {
#include <libavutil/channel_layout.h>
}


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
  const AVCodec *codec = avcodec_find_decoder(codecId);
  if (codec == nullptr) {
    throw std::runtime_error(std::string("No codec found for codec ID: ") + std::string(avcodec_get_name(codecId)));
  }
  spdlog::info("Using codec {} ({})", codec->name, codec->long_name);
  AVDictionary *decoderOptions = nullptr;
  av_dict_set(&decoderOptions, "threads", "auto", 0);
  AVCodecContext *codecContext = avcodec_alloc_context3(codec);
  vivictpp::libav::AVResult ret = avcodec_parameters_to_context(codecContext,
                                            codecParameters);
  if (ret.error()) {
    throw std::runtime_error(std::string("Failed to copy codec parameters to context: ") +
                             ret.getMessage());
  }
  // TODO: avcodec_open2 is not thread safe so should probably be guared in some way
  if (avcodec_open2(codecContext, codec, &decoderOptions) < 0) {
    throw std::runtime_error("Failed to open codec");
  }
  if (codecContext->codec_type == AVMEDIA_TYPE_AUDIO) {
    int channels = getChannels(codecContext);
    std::string channelLayout = getChannelLayout(codecContext);

    spdlog::debug("Audio Codec Context: sample_rate={}, channels={}, channel_layout={}, format={}",
                 codecContext->sample_rate, channels,
                 channelLayout,
                 std::string(av_get_sample_fmt_name(codecContext->sample_fmt)));
  }
  return std::shared_ptr<AVCodecContext>(codecContext, &destroyCodecContext);
}

void vivictpp::libav::destroyCodecContext(AVCodecContext *codecContext) {
    avcodec_free_context(&codecContext);
}
