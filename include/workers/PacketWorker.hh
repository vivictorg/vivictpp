// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef WORKERS_PACKETWORKER_HH
#define WORKERS_PACKETWORKER_HH

#include <atomic>

#include "workers/InputWorker.hh"
#include "workers/PacketQueue.hh"
#include "libav/FormatHandler.hh"
#include "workers/DecoderWorker.hh"
#include "VideoMetadata.hh"
#include "time/Time.hh"
#include "Seeking.hh"

namespace vivictpp {
namespace workers {

class PacketWorker : public InputWorker<int> {
public:
  PacketWorker(std::string source, std::string format = "");
  virtual ~PacketWorker();
  void addDecoderWorker(const std::shared_ptr<DecoderWorker> &decoderWorker);
  void removeDecoderWorker(const std::shared_ptr<DecoderWorker> &decoderWorker);
  bool hasDecoders() { return !decoderWorkers.empty(); };
  int nDecoders() { return decoderWorkers.size(); };
  void seek(vivictpp::time::Time pos, vivictpp::SeekCallback callback);
  const std::vector<VideoMetadata> &getVideoMetadata() {
    std::lock_guard<std::mutex> guard(videoMetadataMutex);
    return this->videoMetadata;
  }
  const std::vector<AVStream *> &getStreams() { return formatHandler.getStreams(); }
  const std::vector<AVStream *> &getVideoStreams() { return formatHandler.getVideoStreams(); }
  const std::vector<AVStream *> &getAudioStreams() { return formatHandler.getAudioStreams(); }

private:
  void doWork() override;
  void setActiveStreams();
  void unrefCurrentPacket();
  void initVideoMetadata();

private:
  vivictpp::libav::FormatHandler formatHandler;
  std::vector<std::shared_ptr<DecoderWorker>> decoderWorkers;
  AVPacket* currentPacket;
  std::vector<VideoMetadata> videoMetadata;
  std::mutex videoMetadataMutex;

};
}  // namespace workers
}  // namespace vivictpp
#endif // WORKERS_PACKETWORKER_HH
