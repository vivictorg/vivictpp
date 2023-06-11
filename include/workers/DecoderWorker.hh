// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef WORKERS_DECODERWORKER_HH
#define WORKERS_DECODERWORKER_HH

#include "workers/InputWorker.hh"
#include "workers/PacketQueue.hh"
#include "libav/FormatHandler.hh"
#include "libav/Filter.hh"
#include "workers/FrameBuffer.hh"
#include "libav/Decoder.hh"
#include "libav/DecoderMetadata.hh"
#include "time/Time.hh"
#include "Seeking.hh"

#include <memory>
#include <queue>
#include <atomic>
#include <string>
#include "spdlog/spdlog.h"


namespace vivictpp {
namespace workers {

class DecoderWorker : public InputWorker<vivictpp::libav::Packet> {
public:
  DecoderWorker(AVStream *stream,
                std::string customFilter = "",
                vivictpp::libav::DecoderOptions decoderOptions = {},
                int frameBufferSize = 50,
                int packetQueueSize = 256);
  virtual ~DecoderWorker();
  void seek(vivictpp::time::Time pos, vivictpp::SeekCallback callback);
  AVStream *getStream() { return stream; };
  AVCodecContext *getCodecContext() { return decoder->getCodecContext(); }
  FrameBuffer &frames() { return frameBuffer; }
  FilteredVideoMetadata getFilteredVideoMetadata() {
    std::shared_ptr<vivictpp::libav::VideoFilter> videoFilter = std::dynamic_pointer_cast<vivictpp::libav::VideoFilter>(filter);
    if (videoFilter) {
      return videoFilter->getFilteredVideoMetadata();
    }
    return FilteredVideoMetadata();
  }
  const vivictpp::libav::DecoderMetadata &getDecoderMetadata() { return decoder->getMetadata(); }
public:
  const int streamIndex;
private:
    bool filterData(const vivictpp::workers::Data<vivictpp::libav::Packet> &data) override {
    return data.data->avPacket()->stream_index == streamIndex;
  };
  bool onData(const vivictpp::workers::Data<vivictpp::libav::Packet> &data) override;
  void doWork() override;
  void dropFrameIfSeekingAndBufferFull();
  bool seeking() { return state == InputWorkerState::SEEKING; }
  void addFrameToBuffer(const vivictpp::libav::Frame &frame);

private:
  AVStream *stream;
  FrameBuffer frameBuffer;

  std::shared_ptr<vivictpp::libav::Decoder> decoder;
  std::shared_ptr<vivictpp::libav::Filter> filter;
  std::queue<vivictpp::libav::Frame> frameQueue;
  vivictpp::time::Time seekPos;
  vivictpp::time::Time lastSeenPts;
  vivictpp::SeekCallback seekCallback;

};
}  // namespace workers
}  // namespace workers

#endif // WORKERS_DECODERWORKER_HH
