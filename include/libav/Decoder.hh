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

namespace vivictpp {
namespace libav {

class Decoder {
public:
  explicit Decoder(AVCodecParameters *codecParameters);
  ~Decoder() = default;

  std::vector<vivictpp::libav::Frame> handlePacket(vivictpp::libav::Packet packet);
  void flush();
  AVCodecContext *getCodecContext() { return this->codecContext.get(); }

private:
  std::shared_ptr<AVCodecContext> codecContext;
  vivictpp::libav::Frame nextFrame;
};

std::shared_ptr<AVCodecContext> createCodecContext(AVCodecParameters *codecParameters);

void destroyCodecContext(AVCodecContext *codecContext);

}  // namespace libav
}  // namespace vivictpp

#endif // LIBAV_DECODER_HH
