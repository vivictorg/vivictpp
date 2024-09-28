// SPDX-FileCopyrightText: 2020 Sveriges Television AB
// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef LIBAV_DECODER_HH
#define LIBAV_DECODER_HH

extern "C" {
#include <libavcodec/avcodec.h>
}

#include <exception>
#include <memory>
#include <stdexcept>
#include <vector>

#include "libav/DecoderMetadata.hh"
#include "libav/DecoderOptions.hh"
#include "libav/Frame.hh"
#include "libav/Packet.hh"
#include "logging/Logging.hh"

namespace vivictpp::libav {

class Decoder {
private:
  std::shared_ptr<AVCodecContext> codecContext;
  Frame nextFrame;
  vivictpp::logging::Logger logger;
  std::shared_ptr<AVBufferRef> hwDeviceContext;
  AVPixelFormat hwPixelFormat;
  AVPixelFormat swPixelFormat;
  AVHWDeviceType hwDeviceType{AV_HWDEVICE_TYPE_NONE};
  DecoderMetadata decoderMetadata;

public:
  explicit Decoder(AVCodecParameters *codecParameters,
                   const DecoderOptions &decoderOptions);
  ~Decoder() = default;

  std::vector<vivictpp::libav::Frame>
  handlePacket(vivictpp::libav::Packet packet);
  void flush();
  AVCodecContext *getCodecContext() { return this->codecContext.get(); }
  AVHWDeviceType getHwDeviceType() { return hwDeviceType; }
  const DecoderMetadata &getMetadata() { return decoderMetadata; }

private:
  void initCodecContext(AVCodecParameters *codecParameters,
                        const DecoderOptions &decoderOptions);
  void initHardwareContext(std::vector<std::string> hwAccels);
  void openCodec();
  void logAudioCodecInfo();
  void selectSwPixelFormat();
};

std::shared_ptr<AVCodecContext>
createCodecContext(AVCodecParameters *codecParameters);

void destroyCodecContext(AVCodecContext *codecContext);

void unrefBuffer(AVBufferRef *bufferRef);

} // namespace vivictpp::libav

#endif // LIBAV_DECODER_HH
