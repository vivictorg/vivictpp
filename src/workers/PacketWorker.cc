// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "workers/PacketWorker.hh"
#include "Seeking.hh"
#include "VideoMetadata.hh"
#include "spdlog/spdlog.h"
#include "time/Time.hh"
#include "workers/DecoderWorker.hh"
#include <stdexcept>
#include <chrono>
#include <thread>

std::shared_ptr<vivictpp::workers::DecoderWorker> findDecoderWorkerForStream(std::vector<std::shared_ptr<vivictpp::workers::DecoderWorker>> decoderWorkers,
                                                                             const AVStream* stream) {
    for (auto const &dw : decoderWorkers) {
        if (dw->getStream() == stream) {
            return dw;
        }
    }
    return nullptr;
}

vivictpp::workers::PacketWorker::PacketWorker(std::string source, std::string format):
    InputWorker<int>(0, "vivictpp::workers::PacketWorker"),
    formatHandler(source, format),
    currentPacket(nullptr) {
    this->initVideoMetadata();
}

vivictpp::workers::PacketWorker::~PacketWorker() {
  unrefCurrentPacket();
  quit();
}

void  vivictpp::workers::PacketWorker::initVideoMetadata() {
    std::vector<VideoMetadata> metadata;
    for (auto const &videoStream : formatHandler.getVideoStreams()) {
        std::shared_ptr<vivictpp::workers::DecoderWorker> decoderWorker = findDecoderWorkerForStream(decoderWorkers, videoStream);
        auto filteredVideoMetadata = decoderWorker ? decoderWorker->getFilteredVideoMetadata() : FilteredVideoMetadata();
        VideoMetadata m =
            VideoMetadata(formatHandler.inputFile, formatHandler.getFormatContext(), videoStream, filteredVideoMetadata);
        metadata.push_back(m);
    }
    {
        std::lock_guard<std::mutex> guard(videoMetadataMutex);
        this->videoMetadata = metadata;
    }
}

void vivictpp::workers::PacketWorker::doWork() {
    if (decoderWorkers.empty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        return;
    }
    logger->trace("vivictpp::workers::PacketWorker::doWork  enter");
    if (currentPacket == nullptr && formatHandler.eof()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        return;
    }
    if (currentPacket == nullptr) {
        currentPacket = formatHandler.nextPacket();
        if (currentPacket != nullptr) {
            logger->debug("Read packet with pts={}", currentPacket->pts);
        }
        if (formatHandler.eof()) {
            logger->debug("End of file reached");
            for (auto dw : decoderWorkers) {
                dw->onEndOfFile();
            }
        }
    }
    if (currentPacket != nullptr) {
        for (auto dw : decoderWorkers) {
            // if any decoder wanted the packet but cannot accept it at this time,
            // we keep the packet and try again later
            vivictpp::workers::Data<vivictpp::libav::Packet> data(
                    new vivictpp::libav::Packet(currentPacket));
            if (!dw->offerData(data, std::chrono::milliseconds(2))) {
                return;
            }
        }
        unrefCurrentPacket();
    }
    logger->trace("vivictpp::workers::PacketWorker::doWork  exit");
}

void vivictpp::workers::PacketWorker::setActiveStreams() {
  std::set<int> activeStreams;
  for (auto dw : decoderWorkers) {
    activeStreams.insert(dw->streamIndex);
  }
  //  logger->debug("vivictpp::workers::PacketWorker::setActiveStreams activeStreams={}", activeStreams);
  formatHandler.setActiveStreams(activeStreams);
}

void vivictpp::workers::PacketWorker::unrefCurrentPacket() {
  if (currentPacket) {
    av_packet_unref(currentPacket);
    currentPacket = nullptr;
  }
}

void vivictpp::workers::PacketWorker::addDecoderWorker(const std::shared_ptr<DecoderWorker> &decoderWorker) {
  _nDecoders++;
  PacketWorker *pw(this);
  sendCommand(new vivictpp::workers::Command([=](uint64_t serialNo) {
        (void) serialNo;
        pw->decoderWorkers.push_back(decoderWorker);
        pw->setActiveStreams();
        pw->initVideoMetadata();
        return true;
      }, "addDecoder"));
}

void vivictpp::workers::PacketWorker::removeDecoderWorker(const std::shared_ptr<DecoderWorker> &decoderWorker) {
  _nDecoders--;
  PacketWorker *pw(this);
  sendCommand(new vivictpp::workers::Command([=](uint64_t serialNo) {
                                               (void) serialNo;
        pw->decoderWorkers.erase(std::remove(pw->decoderWorkers.begin(),
                                             pw->decoderWorkers.end(), decoderWorker),
                                 pw->decoderWorkers.end());
        pw->setActiveStreams();
        pw->initVideoMetadata();
        return true;
      }, "removeDecoder"));
}

void vivictpp::workers::PacketWorker::seek(vivictpp::time::Time pos, vivictpp::SeekCallback callback,
  vivictpp::time::Time streamSeekOffset) {
  PacketWorker *packetWorker(this);
  seeklog->debug("PacketWorker::seek pos={} streamSeekOffset={}", pos, streamSeekOffset);
  sendCommand(new vivictpp::workers::Command([=](uint64_t serialNo) {
      (void) serialNo;
      try {
        packetWorker->formatHandler.seek(pos + streamSeekOffset);
        packetWorker->unrefCurrentPacket();
        for (auto decoderWorker : packetWorker->decoderWorkers) {
          decoderWorker->seek(pos, callback);
        }
      } catch (std::runtime_error &e) {
          callback(vivictpp::time::NO_TIME, true);
      }
      return true;
      }, "seek"));
}
