// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "VideoInputs.hh"
#include "spdlog/spdlog.h"
#include <libavcodec/avcodec.h>

VideoInputs::VideoInputs(VivictPPConfig vivictPPConfig) {
  av_log_set_level(AV_LOG_QUIET);
  for (auto source: vivictPPConfig.sourceConfigs) {
    auto packetWorker = std::shared_ptr<vivictpp::workers::PacketWorker>(
      new vivictpp::workers::PacketWorker(source.path));
    packetWorkers.push_back(packetWorker);
    if (!packetWorker->getVideoStreams().empty()) {
      if (!leftInput.decoder) {
        leftInput.packetWorker = packetWorker;
        leftInput.decoder.reset(
          new vivictpp::workers::DecoderWorker(packetWorker->getVideoStreams()[0], source.filter));
        packetWorker->addDecoderWorker(leftInput.decoder);
        leftInput.decoder->start();
      } else if (!rightInput.decoder) {
        rightInput.packetWorker = packetWorker;
        rightInput.decoder.reset(
          new vivictpp::workers::DecoderWorker(packetWorker->getVideoStreams()[0], source.filter));
        packetWorker->addDecoderWorker(rightInput.decoder);
        rightInput.decoder->start();
      }
    }

    if (!vivictPPConfig.disableAudio && !packetWorker->getAudioStreams().empty()) {
      if (!audio1.decoder) {
        audio1.packetWorker = packetWorker;
        audio1.decoder.reset(
          new vivictpp::workers::DecoderWorker(packetWorker->getAudioStreams()[0]));
        packetWorker->addDecoderWorker(audio1.decoder);
        audio1.decoder->start();
      }
    }

    spdlog::trace("VideoInputs::VideoInputs starting packetWorker");
    packetWorker->start();
  }
}

bool VideoInputs::ptsInRange(double pts) {
  return !isnan(pts) && leftInput.decoder->frames().ptsInRange(pts) &&
    (!rightInput.decoder || rightInput.decoder->frames().ptsInRange(pts));
}

double VideoInputs::duration() {
  double duration = 1e9;
  for( const auto &metadata : leftInput.packetWorker->getVideoMetadata()) {
    if (metadata.duration < duration) {
      duration = metadata.duration;
    }
  }
  if (rightInput.packetWorker) {
    for( const auto &metadata : rightInput.packetWorker->getVideoMetadata()) {
      if (metadata.duration < duration) {
        duration = metadata.duration;
      }
    }
  }
  return duration;
}

double VideoInputs::startTime() {
  return leftInput.packetWorker->getVideoMetadata()[0].startTime;
}

void VideoInputs::stepForward(double pts) {
  leftInput.decoder->frames().stepForward(pts);
  if (rightInput.decoder)
    rightInput.decoder->frames().stepForward(pts);
}

void VideoInputs::stepBackward(double pts) {
  leftInput.decoder->frames().stepBackward(pts);
  if (rightInput.decoder)
    rightInput.decoder->frames().stepBackward(pts);
}

void VideoInputs::dropIfFullAndNextOutOfRange(double currentPts, int framesToDrop) {
  if (currentPts >= leftInput.decoder->frames().maxPts()) {
    leftInput.decoder->frames().dropIfFull(framesToDrop);
  }
  if (rightInput.decoder &&
      ( currentPts >= rightInput.decoder->frames().maxPts())) {
    rightInput.decoder->frames().dropIfFull(framesToDrop);
  }
}

void VideoInputs::dropIfFullAndOutOfRange(double nextPts, int framesToDrop) {
  if (isnan(nextPts) || nextPts > leftInput.decoder->frames().maxPts()) {
    leftInput.decoder->frames().dropIfFull(framesToDrop);
  }
  if (rightInput.decoder &&
      (isnan(nextPts) || nextPts > rightInput.decoder->frames().maxPts())) {
    rightInput.decoder->frames().dropIfFull(framesToDrop);
  }
}

std::array<vivictpp::libav::Frame, 2> VideoInputs::firstFrames() {
  std::array<vivictpp::libav::Frame, 2> result = {leftInput.decoder->frames().first(),
                                                  rightInput.decoder ? rightInput.decoder->frames().first()
                                                  : vivictpp::libav::Frame::emptyFrame()};
  return result;
}

void VideoInputs::seek(double pts) {
  for (auto packetWorker : packetWorkers) {
    packetWorker->seek(pts);
  }
}

std::array<std::vector<VideoMetadata>, 2> VideoInputs::metadata() {
  std::array<std::vector<VideoMetadata>, 2> result = {
    leftInput.packetWorker->getVideoMetadata(),
    rightInput.packetWorker ? rightInput.packetWorker->getVideoMetadata() : std::vector<VideoMetadata>()};
  return result;
}

void VideoInputs::selectVideoStreamLeft(int streamIndex) {
  selectStream(leftInput, streamIndex);
}

void VideoInputs::selectVideoStreamRight(int streamIndex) {
  selectStream(rightInput, streamIndex);
}

void VideoInputs::selectStream(MediaPipe &input, int streamIndex) {
  double currentPts = input.decoder->frames().currentPts();
  input.packetWorker->stop();
  input.packetWorker->removeDecoderWorker(input.decoder);
  input.decoder.reset(
    new vivictpp::workers::DecoderWorker(input.packetWorker->getVideoStreams()[streamIndex]));
  input.packetWorker->addDecoderWorker(input.decoder);
  input.packetWorker->seek(currentPts);
  input.packetWorker->start();
  input.decoder->start();
}
