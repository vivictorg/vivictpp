// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "VideoInputs.hh"
#include "Seeking.hh"
#include "libav/DecoderMetadata.hh"
#include "spdlog/spdlog.h"
#include "time/Time.hh"

extern "C" {
#include <libavcodec/avcodec.h>
}

SeekState::SeekState()
    : logger(vivictpp::logging::getOrCreateLogger("SeekState")) {}

int SeekState::reset(int nSeeks, vivictpp::SeekCallback onFinished) {
  std::lock_guard<std::mutex> lg(m);
  remainingSeeks = nSeeks;
  seekEndPos.clear();
  callback = onFinished;
  seekId++;
  error = false;
  return seekId;
}

void SeekState::handleSeekFinished(int seekId, vivictpp::time::Time seekPos,
                                   bool error) {
  std::lock_guard<std::mutex> lg(m);
  if (seekId != this->seekId)
    return;
  seekEndPos.push_back(seekPos);
  if (error) {
    this->error = true;
  }
  remainingSeeks--;
  logger->debug("handleSeekFinished: remainingSeeks={}", remainingSeeks);
  if (remainingSeeks == 0) {
    vivictpp::time::Time minPos =
        *std::min_element(seekEndPos.begin(), seekEndPos.end());
    callback(minPos, this->error);
  }
}

VideoInputs::VideoInputs()
    : _leftFrameOffset(0), leftPtsOffset(0),
      logger(vivictpp::logging::getOrCreateLogger("VideoInputs")) {}

void VideoInputs::openLeft(const SourceConfig &sourceConfig) {
  // TODO: Need to add audio packet worker for audio support
  packetWorkers.clear();
  leftInput.packetWorker.reset();
  leftInput.decoder.reset();

  videoIndexer.prepareIndex(sourceConfig.path, sourceConfig.formatOptions);

  auto packetWorker = std::shared_ptr<vivictpp::workers::PacketWorker>(
      new vivictpp::workers::PacketWorker(sourceConfig.path,
                                          sourceConfig.formatOptions));
  packetWorkers.push_back(packetWorker);
  if (packetWorker->getVideoStreams().empty()) {
    throw std::runtime_error("No video stream in source" + sourceConfig.path);
  }
  leftInput.packetWorker = packetWorker;
  leftInput.decoder.reset(new vivictpp::workers::DecoderWorker(
      packetWorker->getVideoStreams()[0], sourceConfig.filter,
      {sourceConfig.hwAccels, sourceConfig.preferredDecoders}));
  packetWorker->addDecoderWorker(leftInput.decoder);
  leftInput.decoder->start();
  if (rightInput.packetWorker) {
    packetWorkers.push_back(rightInput.packetWorker);
  }
  packetWorker->start();
};

void VideoInputs::openRight(const SourceConfig &sourceConfig) {
  // TODO: Need to add audio packet worker for audio support
  packetWorkers.clear();
  packetWorkers.push_back(leftInput.packetWorker);
  rightInput.packetWorker.reset();
  rightInput.decoder.reset();
  auto packetWorker = std::shared_ptr<vivictpp::workers::PacketWorker>(
      new vivictpp::workers::PacketWorker(sourceConfig.path,
                                          sourceConfig.formatOptions));
  packetWorkers.push_back(packetWorker);
  if (packetWorker->getVideoStreams().empty()) {
    throw std::runtime_error("No video stream in source" + sourceConfig.path);
  }
  rightInput.packetWorker = packetWorker;
  rightInput.decoder.reset(new vivictpp::workers::DecoderWorker(
      packetWorker->getVideoStreams()[0], sourceConfig.filter,
      {sourceConfig.hwAccels, sourceConfig.preferredDecoders}));
  packetWorker->addDecoderWorker(rightInput.decoder);
  rightInput.decoder->start();
  packetWorker->start();
};

bool VideoInputs::ptsInRange(vivictpp::time::Time pts) {
  return !vivictpp::time::isNoPts(pts) &&
         leftInput.decoder->frames().ptsInRange(pts + leftPtsOffset) &&
         (!rightInput.decoder || rightInput.decoder->frames().ptsInRange(pts));
}

vivictpp::time::Time VideoInputs::duration() {
  vivictpp::time::Time duration = vivictpp::time::NO_TIME;
  for (const auto &metadata : leftInput.packetWorker->getVideoMetadata()) {
    if (metadata.duration != vivictpp::time::NO_TIME &&
        (duration == vivictpp::time::NO_TIME || metadata.duration < duration)) {
      duration = metadata.duration;
    }
  }
  if (rightInput.packetWorker) {
    for (const auto &metadata : rightInput.packetWorker->getVideoMetadata()) {
      if (metadata.duration != vivictpp::time::NO_TIME &&
          (duration == vivictpp::time::NO_TIME ||
           metadata.duration < duration)) {
        duration = metadata.duration;
      }
    }
  }
  return duration;
}

vivictpp::time::Time VideoInputs::startTime() {
  return leftInput.packetWorker->getVideoMetadata()[0].startTime -
         leftPtsOffset;
}

void VideoInputs::step(vivictpp::time::Time pts) {
  leftInput.decoder->frames().step(pts + leftPtsOffset);
  if (rightInput.decoder) {
    rightInput.decoder->frames().step(pts);
  }
}

void VideoInputs::stepForward(vivictpp::time::Time pts) {
  leftInput.decoder->frames().stepForward(pts + leftPtsOffset);
  if (rightInput.decoder) {
    rightInput.decoder->frames().stepForward(pts);
    logger->debug("stepForward Left pts={}, Right pts={}",
                  leftInput.decoder->frames().currentPts() - leftPtsOffset,
                  rightInput.decoder->frames().currentPts());
  } else {
    logger->debug("stepForward Left pts={}",
                  leftInput.decoder->frames().currentPts() - leftPtsOffset);
  }
}

void VideoInputs::stepBackward(vivictpp::time::Time pts) {
  leftInput.decoder->frames().stepBackward(pts + leftPtsOffset);
  if (rightInput.decoder) {
    rightInput.decoder->frames().stepBackward(pts);
    logger->debug("stepBackward Left pts={}, Right pts={}",
                  leftInput.decoder->frames().currentPts() - leftPtsOffset,
                  rightInput.decoder->frames().currentPts());
  } else {
    logger->debug("stepBackward Left pts={}",
                  leftInput.decoder->frames().currentPts() - leftPtsOffset);
  }
}

void VideoInputs::dropIfFullAndNextOutOfRange(vivictpp::time::Time currentPts,
                                              int framesToDrop) {
  if (currentPts == vivictpp::time::NO_TIME ||
      currentPts >= leftInput.decoder->frames().maxPts() - leftPtsOffset) {
    leftInput.decoder->frames().dropIfFull(framesToDrop);
  }
  if (rightInput.decoder &&
      (currentPts == vivictpp::time::NO_TIME ||
       currentPts >= rightInput.decoder->frames().maxPts())) {
    rightInput.decoder->frames().dropIfFull(framesToDrop);
  }
}

void VideoInputs::dropIfFullAndOutOfRange(vivictpp::time::Time nextPts,
                                          int framesToDrop) {
  if (vivictpp::time::isNoPts(nextPts) ||
      nextPts > leftInput.decoder->frames().maxPts() - leftPtsOffset) {
    leftInput.decoder->frames().dropIfFull(framesToDrop);
  }
  if (rightInput.decoder && (vivictpp::time::isNoPts(nextPts) ||
                             nextPts > rightInput.decoder->frames().maxPts())) {
    rightInput.decoder->frames().dropIfFull(framesToDrop);
  }
}

std::array<vivictpp::libav::Frame, 2> VideoInputs::firstFrames() {
  std::array<vivictpp::libav::Frame, 2> result = {
      leftInput.decoder->frames().first(),
      rightInput.decoder ? rightInput.decoder->frames().first()
                         : vivictpp::libav::Frame::emptyFrame()};
  return result;
}

void VideoInputs::seek(vivictpp::time::Time pts,
                       vivictpp::SeekCallback onSeekFinished,
                       vivictpp::time::Time streamSeekOffset) {
  int nDecoders = 0;
  for (auto packetWorker : packetWorkers) {
    nDecoders += packetWorker->nDecoders();
  }
  logger->debug("seek: nDecoders={}", nDecoders);
  int seekId = seekState.reset(nDecoders, onSeekFinished);
  for (auto packetWorker : packetWorkers) {
    if (packetWorker == leftInput.packetWorker) {
      vivictpp::SeekCallback seekCallback =
          [this, seekId](vivictpp::time::Time seekEndPos, bool error) {
            this->seekState.handleSeekFinished(
                seekId, seekEndPos - leftPtsOffset, error);
          };
      packetWorker->seek(pts + leftPtsOffset, seekCallback, streamSeekOffset);
    } else {
      vivictpp::SeekCallback seekCallback =
          [this, seekId](vivictpp::time::Time seekEndPos, bool error) {
            this->seekState.handleSeekFinished(seekId, seekEndPos, error);
          };
      packetWorker->seek(pts, seekCallback, streamSeekOffset);
    }
  }
}

std::array<std::vector<VideoMetadata>, 2> VideoInputs::metadata() {
  std::array<std::vector<VideoMetadata>, 2> result = {
      leftInput.packetWorker->getVideoMetadata(),
      rightInput.packetWorker ? rightInput.packetWorker->getVideoMetadata()
                              : std::vector<VideoMetadata>()};
  return result;
}

std::array<vivictpp::libav::DecoderMetadata, 2> VideoInputs::decoderMetadata() {
  std::array<vivictpp::libav::DecoderMetadata, 2> result = {
      leftInput.decoder->getDecoderMetadata(),
      rightInput.decoder ? rightInput.decoder->getDecoderMetadata()
                         : vivictpp::libav::DecoderMetadata()};
  return result;
}

void VideoInputs::selectVideoStreamLeft(int streamIndex) {
  selectStream(leftInput, streamIndex);
}

void VideoInputs::selectVideoStreamRight(int streamIndex) {
  selectStream(rightInput, streamIndex);
}

void VideoInputs::selectStream(MediaPipe &input, int streamIndex) {
  vivictpp::time::Time currentPts = input.decoder->frames().currentPts();
  input.packetWorker->stop();
  input.packetWorker->removeDecoderWorker(input.decoder);
  input.decoder.reset(new vivictpp::workers::DecoderWorker(
      input.packetWorker->getVideoStreams()[streamIndex]));
  input.packetWorker->addDecoderWorker(input.decoder);
  input.packetWorker->seek(currentPts, [](vivictpp::time::Time _, bool b) {
    (void)_;
    (void)b;
  });
  input.packetWorker->start();
  input.decoder->start();
}
