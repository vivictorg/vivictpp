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
#include <stdexcept>
#include <memory>
#include <vector>

#include "libav/Packet.hh"
#include "libav/Frame.hh"
#include "logging/Logging.hh"

namespace vivictpp {
namespace libav {

struct  DecoderOptions {
  std::string hwAccel;
  std::vector<std::string> preferredDecoders;
};

class Decoder {
public:
  explicit Decoder(AVCodecParameters *codecParameters, const DecoderOptions &decoderOptions);
  ~Decoder() = default;

  std::vector<vivictpp::libav::Frame> handlePacket(vivictpp::libav::Packet packet);
  void flush();
  AVCodecContext *getCodecContext() { return this->codecContext.get(); }
private:
  void initCodecContext(AVCodecParameters *codecParameters, const DecoderOptions &decoderOptions);
  void initHardwareContext(std::string hwAccel);
  void openCodec();
  void logAudioCodecInfo();
private:
  std::shared_ptr<AVCodecContext> codecContext;
  vivictpp::libav::Frame nextFrame;
  vivictpp::logging::Logger logger;
  std::shared_ptr<AVBufferRef> hwDeviceContext;
  AVPixelFormat hwPixelFormat;
};

std::shared_ptr<AVCodecContext> createCodecContext(AVCodecParameters *codecParameters);

void destroyCodecContext(AVCodecContext *codecContext);

void unrefBuffer(AVBufferRef *bufferRef);

}  // namespace libav
}  // namespace vivictpp

#endif // LIBAV_DECODER_HH
