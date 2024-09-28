// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef AUDIO_AUDIOOUTPUT_HH_
#define AUDIO_AUDIOOUTPUT_HH_

#include "AVSync.hh"
#include "libav/Frame.hh"
#include "time/Time.hh"
#include "workers/FrameBuffer.hh"

namespace vivictpp {
namespace audio {

class AudioOutput {
public:
  virtual ~AudioOutput(){};
  virtual vivictpp::time::Time queueDuration() = 0;
  virtual void start() = 0;
  virtual void stop() = 0;
  virtual void queueAudio(const vivictpp::libav::Frame &frame) = 0;
  virtual void clearQueue() = 0;
  virtual vivictpp::time::Time currentPts() = 0;
};

class AudioOutputFactory {
public:
  virtual ~AudioOutputFactory(){};
  virtual std::shared_ptr<AudioOutput> create(AVCodecContext *codecContext) = 0;
};

} // namespace audio
} // namespace vivictpp

#endif // AUDIO_AUDIOOUTPUT_HH_
