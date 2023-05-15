// SPDX-FileCopyrightText: 2020 Sveriges Television AB
// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIDEOINPUTS_HH_
#define VIDEOINPUTS_HH_

#include "time/Time.hh"
extern "C" {
#include <libavformat/avformat.h>
}

#include <memory>
#include <string>
#include <vector>
#include <exception>
#include <atomic>
#include <mutex>

#include "SourceConfig.hh"
#include "VivictPPConfig.hh"
#include "libav/Frame.hh"
#include "workers/PacketWorker.hh"
#include "workers/DecoderWorker.hh"
#include "Seeking.hh"

struct MediaPipe {
    std::shared_ptr<vivictpp::workers::PacketWorker> packetWorker;
    std::shared_ptr<vivictpp::workers::DecoderWorker> decoder;
};

class SeekState {
public:
    SeekState();
    int reset(int nSeeks, vivictpp::SeekCallback onFinished);
    void handleSeekFinished(int seekId, int seekPos, bool error);

private:
    int seekId{1}; // Used to ignore obsolete callbacks from previos seek operations
    std::vector<vivictpp::time::Time> seekEndPos;
    int remainingSeeks;
    vivictpp::SeekCallback callback;
    std::mutex m;
    bool error;
    vivictpp::logging::Logger logger;
};

class VideoInputs {
private:
    std::vector<std::shared_ptr<vivictpp::workers::PacketWorker>> packetWorkers;
    MediaPipe leftInput;
    MediaPipe rightInput;
    MediaPipe audio1;
    int _leftFrameOffset;
    vivictpp::time::Time leftPtsOffset;
    void calcLeftPtsOffset() {
        leftPtsOffset = _leftFrameOffset * leftInput.packetWorker->getVideoMetadata()[0].frameDuration;
        logger->debug("leftPtsOffset: {}", leftPtsOffset);
    }
    SeekState seekState;
    vivictpp::libav::DecoderOptions decoderOptions;
    vivictpp::logging::Logger logger;

public:
    explicit VideoInputs(VivictPPConfig vivictPPConfig);
    void openLeft(std::string source, std::string formatOptions = "");
    void openRight(std::string source, std::string formatOptions = "");
    bool hasLeftSource() { return !!leftInput.packetWorker; }
    bool hasRightSource() { return !!rightInput.packetWorker; }
    bool ptsInRange(vivictpp::time::Time pts);
    void step(vivictpp::time::Time pts);
    void stepForward(vivictpp::time::Time pts);
    void stepBackward(vivictpp::time::Time pts);
    void dropIfFullAndOutOfRange(vivictpp::time::Time nextPts, int framesToDrop);
    void dropIfFullAndNextOutOfRange(vivictpp::time::Time currentPts, int framesToDrop);
    std::array<vivictpp::libav::Frame, 2> firstFrames();
    void seek(vivictpp::time::Time pts, vivictpp::SeekCallback onSeekFinished,
              vivictpp::time::Time streamSeekOffset = 0);
    std::array<std::vector<VideoMetadata>, 2> metadata();
    vivictpp::time::Time duration();
    vivictpp::time::Time startTime();
    vivictpp::time::Time frameDuration() {
      if (hasLeftSource()) {
        return vivictpp::time::NO_TIME;
      }
      vivictpp::time::Time frameDuration = metadata()[0][0].frameDuration;
      if (hasRightSource()) {
        frameDuration = std::min(frameDuration, metadata()[1][0].frameDuration);
      }
      return frameDuration;
    }
    bool hasMaxPts() {
        return leftInput.packetWorker->getVideoMetadata()[0].hasDuration()
            || (rightInput.packetWorker &&
                rightInput.packetWorker->getVideoMetadata()[0].hasDuration());
    }
    vivictpp::time::Time minPts() {
        VideoMetadata meta1 = leftInput.packetWorker->getVideoMetadata()[0];
        if (!rightInput.packetWorker) {
            return meta1.startTime - leftPtsOffset;
        }
        VideoMetadata meta2 = rightInput.packetWorker->getVideoMetadata()[0];
        return std::max(meta1.startTime - leftPtsOffset, meta2.startTime);
    }
    vivictpp::time::Time maxPts() {
        VideoMetadata meta1 = leftInput.packetWorker->getVideoMetadata()[0];
        if (!meta1.hasDuration()) {
          return vivictpp::time::NO_TIME;
        }
        if (!rightInput.packetWorker) {
            return meta1.endTime - leftPtsOffset;
        }
        VideoMetadata meta2 = rightInput.packetWorker->getVideoMetadata()[0];
        return std::min(meta1.endTime - leftPtsOffset, meta2.endTime);
    }
        int leftFrameOffset() { return _leftFrameOffset; }
    int increaseLeftFrameOffset() {
        _leftFrameOffset++;
        calcLeftPtsOffset();
        return _leftFrameOffset;
    }
    int decreaseLeftFrameOffset() {
         _leftFrameOffset--;
         calcLeftPtsOffset();
         return _leftFrameOffset;
    }

    vivictpp::time::Time nextPts() {
      vivictpp::time::Time nextPtsL = leftInput.decoder->frames().nextPts() - leftPtsOffset;
      if (!rightInput.decoder) {
        return nextPtsL;
      }
      vivictpp::time::Time nextPtsR = rightInput.decoder->frames().nextPts();
      if (vivictpp::time::isNoPts(nextPtsL)) {
        return nextPtsL;
      }
      if (vivictpp::time::isNoPts(nextPtsR)) {
        return nextPtsR;
      }
      return std::min(nextPtsL, nextPtsR);
    }
    vivictpp::time::Time previousPts() {
      vivictpp::time::Time ppl = leftInput.decoder->frames().previousPts() - leftPtsOffset;
      if(!rightInput.decoder) {
        return ppl;
      }
      vivictpp::time::Time ppr = rightInput.decoder->frames().previousPts();
      if (vivictpp::time::isNoPts(ppl)) {
        return ppl;
      }
      if (vivictpp::time::isNoPts(ppr)) {
        return ppr;
      }
      return std::max(ppl, ppr);
    }
    void selectVideoStreamLeft(int streamIndex);
    void selectVideoStreamRight(int streamIndex);
    bool hasAudio() { return audio1.decoder.get() != nullptr;  }
    AVCodecContext *getAudioCodecContext() {
        if (!audio1.decoder) {
            throw new std::runtime_error("Input has no audio");
        }
        return audio1.decoder->getCodecContext();
    }
    vivictpp::workers::FrameBuffer &audioFrames() {
    if (!audio1.decoder) {
        throw new std::runtime_error("Input has no audio");
    }
        return audio1.decoder->frames();
}

private:
void selectStream(MediaPipe &input, int streamIndex);
};

#endif // VIDEOINPUTS_HH_
