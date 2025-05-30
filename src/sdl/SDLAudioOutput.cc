// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "sdl/SDLAudioOutput.hh"

#include "libav/Utils.hh"

extern "C" {
#include <SDL3/SDL.h>
#include <libavutil/avutil.h>
}

#include "spdlog/spdlog.h"
#include <exception>

vivictpp::sdl::SDLAudioOutput::SDLAudioOutput(AVCodecContext *codecContext)
    : sampleFormat(codecContext->sample_fmt),
      channels(vivictpp::libav::getChannels(codecContext)),
      bytesPerSample(av_get_bytes_per_sample(sampleFormat) * channels) {
  SDL_AudioSpec wantedSpec;
  wantedSpec.channels = vivictpp::libav::getChannels(codecContext);
  wantedSpec.freq = codecContext->sample_rate;
  wantedSpec.format = SDL_AUDIO_S16;
  wantedSpec.samples = 1024;
  wantedSpec.silence = 0;
  wantedSpec.callback = nullptr;
  wantedSpec.userdata = nullptr;

  audioDevice = SDL_OpenAudioDevice(nullptr, 0, &wantedSpec, &obtainedSpec,
                                    SDL_AUDIO_ALLOW_FREQUENCY_CHANGE |
                                        SDL_AUDIO_ALLOW_CHANNELS_CHANGE);
  if (audioDevice <= 0) {
    throw std::runtime_error("Failed to open audio device");
  }
}

vivictpp::sdl::SDLAudioOutput::~SDLAudioOutput() {
  SDL_CloseAudioDevice(audioDevice);
}

void vivictpp::sdl::SDLAudioOutput::queueAudio(
    const vivictpp::libav::Frame &frame) {
  uint32_t size = (uint32_t)av_samples_get_buffer_size(
      NULL, vivictpp::libav::getChannels(frame.avFrame()),
      frame.avFrame()->nb_samples, (AVSampleFormat)frame.avFrame()->format, 1);
  SDL_QueueAudio(audioDevice, frame.avFrame()->data[0], size);
  lastPts =
      av_rescale(frame.pts(), vivictpp::time::TIME_BASE, obtainedSpec.freq);
}

void vivictpp::sdl::SDLAudioOutput::clearQueue() {
  SDL_ClearQueuedAudio(audioDevice);
}

vivictpp::time::Time vivictpp::sdl::SDLAudioOutput::currentPts() {
  return lastPts - queueDuration();
}

uint32_t vivictpp::sdl::SDLAudioOutput::queuedSamples() {
  uint32_t queueSize = SDL_GetQueuedAudioSize(audioDevice);
  return queueSize / bytesPerSample;
}

vivictpp::time::Time vivictpp::sdl::SDLAudioOutput::queueDuration() {
  return av_rescale(queuedSamples(), vivictpp::time::TIME_BASE,
                    obtainedSpec.freq);
}

void vivictpp::sdl::SDLAudioOutput::start() {
  SDL_PauseAudioDevice(audioDevice, 0);
}

void vivictpp::sdl::SDLAudioOutput::stop() {
  SDL_PauseAudioDevice(audioDevice, 1);
}
