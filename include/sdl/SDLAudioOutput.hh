// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef AUDIO_OUTPUT_HH_
#define AUDIO_OUTPUT_HH_

#include "sdl/SDLUtils.hh"
#include "workers/FrameBuffer.hh"
#include "AVSync.hh"
#include "audio/AudioOutput.hh"
#include "time/Time.hh"
#include <memory>

extern "C" {
  #include <libavcodec/avcodec.h>
}

namespace vivictpp {
namespace sdl {

struct AudioBuffer {
  uint8_t *data{nullptr};
  int size{0};
  int index{0};
  int64_t pts{0};
};


class SDLAudioOutput : public vivictpp::audio::AudioOutput  {
public:
  SDLAudioOutput(AVCodecContext *codecContext);
  ~SDLAudioOutput();
  void start() override;
  void stop() override;
  void queueAudio(const vivictpp::libav::Frame &frame) override;
  void clearQueue() override;
  vivictpp::time::Time currentPts() override;
  uint32_t queuedSamples();
  vivictpp::time::Time queueDuration() override;
private:
  void nextFrame();
private:
  vivictpp::time::Time lastPts;
  SDLInitializer sdlInitializer;
  SDL_AudioSpec obtainedSpec;
  SDL_AudioDeviceID audioDevice;
  AudioBuffer audioBuffer;
  AVSampleFormat sampleFormat;
  int channels;
  int bytesPerSample;
};

class SDLAudioOutputFactory : public vivictpp::audio::AudioOutputFactory {
public:
  virtual std::shared_ptr<vivictpp::audio::AudioOutput> create(AVCodecContext *codecContext) override {
      return std::make_shared<vivictpp::sdl::SDLAudioOutput>(codecContext);
    }

};

static SDLAudioOutputFactory audioOutputFactory;

}  // sdl
}  // vivictpp



#endif
