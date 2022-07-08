// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "workers/DecoderWorker.hh"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"


std::string filterStr(std::string stdFilter, std::string customFilter) {
  if (customFilter.empty()) {
    return stdFilter;
  } else {
    return customFilter + "," + stdFilter;
  }
}

vivictpp::libav::Filter *createFilter(AVStream *stream, AVCodecContext *codecContext, std::string customFilter) {
  switch (stream->codecpar->codec_type) {
  case AVMEDIA_TYPE_VIDEO:
    return new vivictpp::libav::VideoFilter(stream, codecContext, filterStr("format=yuv420p", customFilter));
  case AVMEDIA_TYPE_AUDIO:
    return new vivictpp::libav::AudioFilter(codecContext, "aformat=sample_fmts=s16");
  default:
    throw std::runtime_error("Filter not supported for codec_type");
  }
}

vivictpp::workers::DecoderWorker::DecoderWorker(AVStream *stream,
                                                std::string customFilter,
                                                int frameBufferSize,
                                                int packetQueueSize) :
  InputWorker(packetQueueSize, "DecoderWorker"),
  streamIndex(stream->index),
  stream(stream),
  frameBuffer(frameBufferSize),
  decoder(new vivictpp::libav::Decoder(stream->codecpar)),
  filter(createFilter(stream, decoder->getCodecContext(), customFilter)),
  lastSeenPts(AV_NOPTS_VALUE)
{}

vivictpp::workers::DecoderWorker::~DecoderWorker() {
  quit();
}


void vivictpp::workers::DecoderWorker::seek(vivictpp::time::Time pos, vivictpp::SeekCallback callback) {
  seeklog->debug("vivictpp::workers::DecoderWorker::seek pos={}", pos);
  DecoderWorker *dw(this);
  sendCommand(new vivictpp::workers::Command([=](uint64_t serialNo) {
        dw->messageQueue.clearDataOlderThan(serialNo);
        dw->state = InputWorkerState::SEEKING;
        dw->decoder->flush();
        dw->frameBuffer.clear();
        dw->seekPos = pos;
        dw->seekCallback = callback;
        return true;
                                             }, "seek"));
}

void logPacket(vivictpp::libav::Packet pkt, const std::shared_ptr<spdlog::logger> &logger) {
  AVPacket *packet = pkt.avPacket();
  if (packet) {
    logger->trace("Packet: size={} data={} pts={}", packet->size, packet->data, packet->pts);
  } else {
    logger->trace("Packet: nullptr");
  }
}

void vivictpp::workers::DecoderWorker::doWork() {
    logger->trace("vivictpp::workers::DecoderWorker::doWork");
    while (!frameQueue.empty() && frameBuffer.waitForNotFull(std::chrono::milliseconds(2))) {
      dropFrameIfSeekingAndBufferFull();
      addFrameToBuffer(frameQueue.front());
      frameQueue.pop();
    }
}

bool vivictpp::workers::DecoderWorker::onData(const vivictpp::workers::Data<vivictpp::libav::Packet> &data) {
  if (!frameQueue.empty()) {
    return false;
  }
  if (!seeking() && !frameBuffer.waitForNotFull(std::chrono::milliseconds(2))) {
    logger->trace("vivictpp::workers::DecoderWorker::onData frameBuffer full");
    return false;
  }
  // TODO: check filter.eof

  vivictpp::libav::Packet packet = *(data.data);
  logPacket(packet, logger);
  std::vector<vivictpp::libav::Frame> frames = decoder->handlePacket(packet.avPacket());
  for (auto frame : frames) {
    dropFrameIfSeekingAndBufferFull();
    vivictpp::libav::Frame filtered = filter ? filter->filterFrame(frame) : frame;
    if (!filtered.empty()) {
      if (frameBuffer.isFull()) {
        frameQueue.push(filtered);
      } else {
        addFrameToBuffer(filtered);
      }
    }
  }
  return true;
}

void inline vivictpp::workers::DecoderWorker::dropFrameIfSeekingAndBufferFull() {
  if (seeking()) {
    seeklog->debug("vivictpp::workers::DecoderWorker::dropFrameIfSeekingAndBufferFull Dropping 1 frame from buffer");
    frameBuffer.dropIfFull(1);
  }
}

void vivictpp::workers::DecoderWorker::addFrameToBuffer(const vivictpp::libav::Frame &frame) {
    logger->debug("pts={} AV_NOPTS_VALUE={}", frame.pts(), AV_NOPTS_VALUE);
    vivictpp::time::Time pts = frame.pts();
    if (pts == AV_NOPTS_VALUE) {
      if (lastSeenPts == AV_NOPTS_VALUE) {
        pts = 0;
      } else {
        pts = lastSeenPts + av_rescale(vivictpp::time::TIME_BASE, stream->r_frame_rate.den, stream->r_frame_rate.num);
      }
      logger->warn("DecoderWorker::doWork Frame has no pts, estimating pts {}", pts);
    } else {
      pts = av_rescale_q(pts, stream->time_base, vivictpp::time::TIME_BASE_Q);
    }
    lastSeenPts = pts;
    logger->debug("DecoderWorker::doWork Buffering frame with pts={}s ({})",
                  pts, frame.pts());
    frameBuffer.write(frame, pts);
    if(seeking()) {
      seeklog->debug("vivictpp::workers::DecoderWorker::addFrameToBuffer written pts={} seekPos={}", pts, seekPos);
      if (pts >= seekPos) {
        seeklog->debug("DecoderWorker::doWork seekFinished", pts);
        this->seekCallback(pts);
        this->state = InputWorkerState::ACTIVE;
      }
    }
}
