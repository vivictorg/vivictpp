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
    bool ptsInRange(double pts);
    void stepForward(double pts);
    void stepBackward(double pts);
    void dropIfFullAndOutOfRange(double nextPts, int framesToDrop);
    void dropIfFullAndNextOutOfRange(double currentPts, int framesToDrop);
    std::array<vivictpp::libav::Frame, 2> firstFrames();
    void seek(double pts);
    std::array<std::vector<VideoMetadata>, 2> metadata();
    double duration();
    double startTime();
    double nextPts() {
      double nextPtsL = leftInput.decoder->frames().nextPts();
      if (!rightInput.decoder) {
        return nextPtsL;
      }
      double nextPtsR = rightInput.decoder->frames().nextPts();
      if (isnan(nextPtsL)) {
        return nextPtsL;
      }
      if (isnan(nextPtsR)) {
        return nextPtsR;
      }
      return std::min(nextPtsL, nextPtsR);
    }
    double previousPts() {
      double ppl = leftInput.decoder->frames().previousPts();
      if(!rightInput.decoder) {
        return ppl;
      }
      double ppr = rightInput.decoder->frames().previousPts();
      if (isnan(ppl)) {
        return ppl;
      }
      if (isnan(ppr)) {
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
