// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef AUDIO_AUDIOOUTPUT_HH_
#define AUDIO_AUDIOOUTPUT_HH_

#include "libav/Frame.hh"
#include "workers/FrameBuffer.hh"
#include "AVSync.hh"

namespace vivictpp {
namespace audio {

class AudioOutput {
public:
  virtual ~AudioOutput(){};
  virtual double queueDuration() = 0;
  virtual void start() = 0;
  virtual void stop() = 0;
  virtual void queueAudio(const vivictpp::libav::Frame &frame) = 0;
  virtual void clearQueue() = 0;
  virtual  double currentPts() = 0;

};

class AudioOutputFactory {
public:
  virtual ~AudioOutputFactory(){};
  virtual std::shared_ptr<AudioOutput> create(AVCodecContext *codecContext) = 0;
};

}  // audio
}  // vivictpp

#endif  // AUDIO_AUDIOOUTPUT_HH_
