// SPDX-FileCopyrightText: 2020 Sveriges Television AB
// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIDEOINPUTS_HH_
#define VIDEOINPUTS_HH_

extern "C" {
#include <libavformat/avformat.h>
}

#include <memory>
#include <string>
#include <vector>
#include <exception>

#include "SourceConfig.hh"
#include "VivictPPConfig.hh"
#include "libav/Frame.hh"
#include "workers/PacketWorker.hh"
#include "workers/DecoderWorker.hh"

struct MediaPipe {
    std::shared_ptr<vivictpp::workers::PacketWorker> packetWorker;
    std::shared_ptr<vivictpp::workers::DecoderWorker> decoder;
};

class VideoInputs {
private:
    std::vector<std::shared_ptr<vivictpp::workers::PacketWorker>> packetWorkers;
    MediaPipe leftInput;
    MediaPipe rightInput;
    MediaPipe audio1;

public:
    explicit VideoInputs(VivictPPConfig vivictPPConfig);
    bool ptsInRange(vivictpp::time::Time pts);
    void stepForward(vivictpp::time::Time pts);
    void stepBackward(vivictpp::time::Time pts);
    void dropIfFullAndOutOfRange(vivictpp::time::Time nextPts, int framesToDrop);
    void dropIfFullAndNextOutOfRange(vivictpp::time::Time currentPts, int framesToDrop);
    std::array<vivictpp::libav::Frame, 2> firstFrames();
    void seek(vivictpp::time::Time pts);
    std::array<std::vector<VideoMetadata>, 2> metadata();
    vivictpp::time::Time duration();
    vivictpp::time::Time startTime();
    vivictpp::time::Time minPts() {
        VideoMetadata meta1 = leftInput.packetWorker->getVideoMetadata()[0];
        if (!rightInput.packetWorker) {
            return meta1.startTime;
        }
        VideoMetadata meta2 = rightInput.packetWorker->getVideoMetadata()[0];
        return std::max(meta1.startTime, meta2.startTime);
    }
    vivictpp::time::Time maxPts() {
        VideoMetadata meta1 = leftInput.packetWorker->getVideoMetadata()[0];
        if (!rightInput.packetWorker) {
            return meta1.endTime;
        }
        VideoMetadata meta2 = rightInput.packetWorker->getVideoMetadata()[0];
        return std::min(meta1.endTime, meta2.endTime);
    }
    vivictpp::time::Time nextPts() {
      vivictpp::time::Time nextPtsL = leftInput.decoder->frames().nextPts();
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
      vivictpp::time::Time ppl = leftInput.decoder->frames().previousPts();
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
