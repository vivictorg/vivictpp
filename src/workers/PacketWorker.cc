// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "workers/PacketWorker.hh"
#include "spdlog/spdlog.h"

vivictpp::workers::PacketWorker::PacketWorker(std::string source):
    InputWorker<int>(0, "PacketWorker"),
  formatHandler(source),
  currentPacket(nullptr)
{
    for (auto const &videoStream : formatHandler.getVideoStreams()) {
    VideoMetadata m =
        VideoMetadata(source, formatHandler.getFormatContext(), videoStream);
    this->videoMetadata.push_back(m);
  }
}

vivictpp::workers::PacketWorker::~PacketWorker() {
  unrefCurrentPacket();
  quit();
}

void vivictpp::workers::PacketWorker::doWork() {
  if (!hasDecoders()) {
     usleep(5 * 1000);
     return;
  }
  logger->trace("vivictpp::workers::PacketWorker::doWork  enter");
  if (currentPacket == nullptr) {
    currentPacket = formatHandler.nextPacket();
  }
  if (currentPacket == nullptr) {
    logger->trace("Packet is null, eof reached");
    usleep(5 * 1000);
  } else {
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
  PacketWorker *pw(this);
  sendCommand(new vivictpp::workers::Command([=](uint64_t serialNo) {
        (void) serialNo;
        pw->decoderWorkers.push_back(decoderWorker);
        pw->setActiveStreams();
        return true;
      }, "addDecoder"));
}

void vivictpp::workers::PacketWorker::removeDecoderWorker(const std::shared_ptr<DecoderWorker> &decoderWorker) {
  PacketWorker *pw(this);
  sendCommand(new vivictpp::workers::Command([=](uint64_t serialNo) {
                                               (void) serialNo;
        pw->decoderWorkers.erase(std::remove(pw->decoderWorkers.begin(),
                                             pw->decoderWorkers.end(), decoderWorker),
                                 pw->decoderWorkers.end());
        pw->setActiveStreams();
        return true;
      }, "removeDecoder"));
}

void vivictpp::workers::PacketWorker::seek(double pos) {
  PacketWorker *packetWorker(this);
  sendCommand(new vivictpp::workers::Command([=](uint64_t serialNo) {
                                               (void) serialNo;
        packetWorker->formatHandler.seek(pos);
        packetWorker->unrefCurrentPacket();
        for (auto decoderWorker : packetWorker->decoderWorkers) {
          decoderWorker->seek(pos);
        }
        return true;
      }, "seek"));
}

