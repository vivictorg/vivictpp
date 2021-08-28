// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef AUDIO_OUTPUT_HH_
#define AUDIO_OUTPUT_HH_

#include "sdl/SDLUtils.hh"
#include "workers/FrameBuffer.hh"
#include "AVSync.hh"

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

class AudioOutput  {
public:
    AudioOutput(AVCodecContext *codecContext, vivictpp::workers::FrameBuffer &frameBuffer, AVSync &avSync);
  ~AudioOutput();
  void start();
  void stop();
  void queueAudio(const vivictpp::libav::Frame &frame);
  void clearQueue();
  double currentPts();
  uint32_t queuedSamples();
  double queueDuration();
  void sdlCallback(uint8_t *stream, int len);
  static void sdlStaticCallback(void *opaque, uint8_t *stream, int len);
private:
  void nextFrame();
private:
  int64_t lastPts;
  SDLInitializer sdlInitializer;
  vivictpp::workers::FrameBuffer &frameBuffer;
  AVSync &avSync;
  SDL_AudioSpec obtainedSpec;
  SDL_AudioDeviceID audioDevice;
  AudioBuffer audioBuffer;
  vivictpp::libav::Frame currentFrame;
  AVSampleFormat sampleFormat;
  int channels;
  int bytesPerSample;
};
};  // sdl
};  // vivictpp


#endif
