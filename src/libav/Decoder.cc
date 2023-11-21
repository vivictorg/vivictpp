// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "libav/Decoder.hh"
#include "libav/AVErrorUtils.hh"
#include "libav/Utils.hh"
#include <set>

extern "C" {
#include <libavcodec/codec.h>
#include <libavutil/pixfmt.h>
#include <libavutil/channel_layout.h>
}


#include "spdlog/spdlog.h"

static enum AVPixelFormat getHwFormat(AVCodecContext *ctx,
                                      const enum AVPixelFormat *pix_fmts) {
  const AVPixelFormat *wantedPixelFormat = (AVPixelFormat*) ctx->opaque;

  const enum AVPixelFormat *p;

  for (p = pix_fmts; *p != -1; p++) {
    if (*p == *wantedPixelFormat)
      return *p;
  }

  spdlog::warn("Failed to get hw surface format");
  return AV_PIX_FMT_NONE;
}

vivictpp::libav::Decoder::Decoder(AVCodecParameters *codecParameters,
                                  const DecoderOptions &decoderOptions)
    : codecContext(nullptr),
      logger(vivictpp::logging::getOrCreateLogger("vivictpp::libav::Decoder")),
      hwDeviceContext(nullptr),
      hwPixelFormat(AV_PIX_FMT_NONE),
      swPixelFormat(AV_PIX_FMT_NONE){
  initCodecContext(codecParameters, decoderOptions);
  initHardwareContext(decoderOptions.hwAccels);
  openCodec();
}

const AVCodec* findDecoder(AVCodecID codecId, const vivictpp::libav::DecoderOptions &decoderOptions) {
  for (const auto &codecName: decoderOptions.preferredDecoders) {
    const AVCodec *codec = avcodec_find_decoder_by_name(codecName.c_str());
    if (codec && codec->id == codecId) {
      return codec;
    }

  }
  return avcodec_find_decoder(codecId);
}

void vivictpp::libav::Decoder::initCodecContext(AVCodecParameters *codecParameters, const DecoderOptions &decoderOptions) {
  AVCodecID codecId = codecParameters->codec_id;
  const AVCodec *codec = findDecoder(codecId, decoderOptions);
  if (codec == nullptr) {
    throw std::runtime_error(std::string("No codec found for codec ID: ") + std::string(avcodec_get_name(codecId)));
  }
  decoderMetadata.name = codec->name;
  logger->info("Using codec {} ({})", codec->name, codec->long_name);
  AVCodecContext *codecContext = avcodec_alloc_context3(codec);
  vivictpp::libav::AVResult ret = avcodec_parameters_to_context(codecContext,
                                            codecParameters);
  ret.throwOnError("Failed to copy codec parameters to context");

  this->codecContext.reset(codecContext, &destroyCodecContext);
}

std::set<AVHWDeviceType> supportedDeviceTypes() {
  std::set<AVHWDeviceType> types;
  AVHWDeviceType type = AV_HWDEVICE_TYPE_NONE;
  while((type = av_hwdevice_iterate_types(type)) != AV_HWDEVICE_TYPE_NONE) {
    types.insert(type);
  }
  return types;
}

bool deviceTypeSupported(AVHWDeviceType type) {
  return supportedDeviceTypes().count(type) > 0;
}

void logSupportedDeviceTypes() {
  auto logger = vivictpp::logging::getOrCreateLogger("vivictpp::libav::Decoder");
  logger->warn("Supported device types:");
  for (auto &type : supportedDeviceTypes()) {
    logger->warn("  {}", av_hwdevice_get_type_name(type));
  }
}

void vivictpp::libav::Decoder::initHardwareContext(std::vector<std::string> hwAccels) {
  if (hwAccels.empty()) {
    return;
  }

  std::vector<AVHWDeviceType> candidateTypes;
  for (auto &hwAccel : hwAccels) {
    AVHWDeviceType type = av_hwdevice_find_type_by_name(hwAccel.c_str());
    if (type == AV_HWDEVICE_TYPE_NONE) {
      logger->warn("Unknown hardware device type: {}", hwAccel);
      logSupportedDeviceTypes();
      return;
    }

    if (deviceTypeSupported(type)) {
      candidateTypes.push_back(type);
    } else {
      logger->warn("Device type not supported: {}", hwAccel);
      logSupportedDeviceTypes();
    }
  }

  for (auto &type : candidateTypes) {
    for (size_t i = 0;; i++) {
      const AVCodecHWConfig *config = avcodec_get_hw_config(codecContext->codec, i);
      if (!config) {
        break;
      }
      if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX &&
          config->device_type == type) {
        AVBufferRef *hwDeviceContext;
        vivictpp::libav::AVResult ret = av_hwdevice_ctx_create(&hwDeviceContext, type,
                                                               NULL, NULL, 0);
        if (ret.error()) {
          logger->info("Failed to create hardware device of type {}", av_hwdevice_get_type_name(type));
        } else {
          char buf[512];
          logger->info("Using hardware device of type {}, HW pixel format: {}",
                       av_hwdevice_get_type_name(type),
                       av_get_pix_fmt_string(buf, 512, config->pix_fmt));
          this->decoderMetadata.hwAccel = av_hwdevice_get_type_name(type);
          this->decoderMetadata.hwPixelFormat = av_get_pix_fmt_name(config->pix_fmt);
          this->hwPixelFormat = config->pix_fmt;
          this->codecContext->opaque = (void*) &(this->hwPixelFormat);
          this->hwDeviceType = type;
          this->codecContext->get_format = getHwFormat;
          this->hwDeviceContext.reset(hwDeviceContext, &unrefBuffer);
          this->codecContext->hw_device_ctx = av_buffer_ref(this->hwDeviceContext.get());

          return;
        }
      }
    }

    logger->debug("Decoder {} does not support device type {}.",
                  this->codecContext.get()->codec->name,
                  av_hwdevice_get_type_name(type));
  }
  logger->info("No hardware device found for codec {}", this->codecContext.get()->codec->name);
}

void vivictpp::libav::Decoder::openCodec() {
  AVDictionary *decoderOptions = nullptr;
  av_dict_set(&decoderOptions, "threads", "auto", 0);
  vivictpp::libav::AVResult ret = avcodec_open2(codecContext.get(), codecContext->codec, &decoderOptions);
  ret.throwOnError("Failed to open codec");
  logAudioCodecInfo();
}

void vivictpp::libav::Decoder::logAudioCodecInfo() {
  if (codecContext->codec_type == AVMEDIA_TYPE_AUDIO) {
    int channels = getChannels(codecContext.get());
    std::string channelLayout = getChannelLayout(codecContext.get());

    logger->info("Audio Codec Context: sample_rate={}, channels={}, channel_layout={}, format={}",
                 codecContext->sample_rate, channels,
                 channelLayout,
                 std::string(av_get_sample_fmt_name(codecContext->sample_fmt)));
  }
}

void vivictpp::libav::Decoder::flush() { avcodec_flush_buffers(this->codecContext.get()); }

std::vector<vivictpp::libav::Frame> vivictpp::libav::Decoder::handlePacket(Packet packet) {
  logger->trace("handlePacket");
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

static const std::vector<AVPixelFormat> preferredPixelFormats = {AV_PIX_FMT_YUV420P, AV_PIX_FMT_YUV420P10, AV_PIX_FMT_NV12, AV_PIX_FMT_P010};

void vivictpp::libav::Decoder::selectSwPixelFormat() {
  std::vector<AVPixelFormat> availableTargetFormats;
  enum AVPixelFormat *pixelFormats;
  av_hwframe_transfer_get_formats(nextFrame.avFrame()->hw_frames_ctx, AV_HWFRAME_TRANSFER_DIRECTION_FROM,
                                  &pixelFormats, 0);

  for(enum AVPixelFormat *curr = pixelFormats; *curr != AV_PIX_FMT_NONE; curr++) {
    availableTargetFormats.push_back(*curr);
    logger->info("possible target format: {}", av_get_pix_fmt_name(*curr));
  }
  av_free(pixelFormats);
  for(const auto &format : preferredPixelFormats) {
    if(std::find(availableTargetFormats.begin(), availableTargetFormats.end(), format) != availableTargetFormats.end()) {
      this->swPixelFormat = format;
      return;
    }
  }
  this->swPixelFormat = availableTargetFormats[0];
}

void vivictpp::libav::destroyCodecContext(AVCodecContext *codecContext) {
  avcodec_free_context(&codecContext);
}

void vivictpp::libav::unrefBuffer(AVBufferRef *bufferRef) {
  av_buffer_unref(&bufferRef);
}
