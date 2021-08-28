// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "sdl/AudioOutput.hh"

extern "C" {
  #include <SDL.h>
}

#include "spdlog/spdlog.h"
#include <exception>

vivictpp::sdl::AudioOutput::AudioOutput(AVCodecContext *codecContext,
                                        vivictpp::workers::FrameBuffer &frameBuffer,
                                        AVSync &avSync) :
  frameBuffer(frameBuffer),
  avSync(avSync),
  sampleFormat(codecContext->sample_fmt),
  channels(codecContext->channels),
  bytesPerSample(av_get_bytes_per_sample(sampleFormat) * channels) {
  SDL_AudioSpec wantedSpec;
  wantedSpec.channels = codecContext->channels;
  wantedSpec.freq = codecContext->sample_rate;
  wantedSpec.format = AUDIO_S16SYS;
  wantedSpec.samples = 1024;
  wantedSpec.silence = 0;
  wantedSpec.callback = nullptr;
  wantedSpec.userdata = nullptr;

  audioDevice = SDL_OpenAudioDevice(nullptr, 0, &wantedSpec, &obtainedSpec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE);
  if (audioDevice <= 0) {
    throw std::runtime_error("Failed to open audio device");
  }
}

vivictpp::sdl::AudioOutput::~AudioOutput() {
  SDL_CloseAudioDevice(audioDevice);
}

void vivictpp::sdl::AudioOutput::queueAudio(const vivictpp::libav::Frame &frame) {
  uint32_t size = (uint32_t) av_samples_get_buffer_size(NULL, frame.avFrame()->channels,
                                                frame.avFrame()->nb_samples,
                                                (AVSampleFormat) frame.avFrame()->format, 1);
  SDL_QueueAudio(audioDevice, frame.avFrame()->data[0], size);
  lastPts = frame.pts(); // + frame.avFrame()->nb_samples + 1;
}

void vivictpp::sdl::AudioOutput::clearQueue() {
  SDL_ClearQueuedAudio(audioDevice);
}

double vivictpp::sdl::AudioOutput::currentPts() {
  return (lastPts - queuedSamples()) / (double) obtainedSpec.freq;
}

uint32_t vivictpp::sdl::AudioOutput::queuedSamples() {
  uint32_t queueSize = SDL_GetQueuedAudioSize(audioDevice);
  return queueSize / bytesPerSample;
}

double vivictpp::sdl::AudioOutput::queueDuration() {
  return queuedSamples() / (double) obtainedSpec.freq;
}

void vivictpp::sdl::AudioOutput::nextFrame() {
  spdlog::debug("AudioOutput::nextFrame  enter");
  double nextPts = frameBuffer.nextPts();
  spdlog::debug("AudioOutput::nextFrame  nextPts={}", nextPts);
  while (isnan(nextPts)) {
    usleep(2000);
    nextPts = frameBuffer.nextPts();
    spdlog::info("AudioOutput::nextFrame  nextPts={}", nextPts);
  }
  frameBuffer.stepForward(nextPts);
  currentFrame = frameBuffer.first();
  spdlog::info("AudioOutput::nextFrame  currentFrame.pts={} nb_samples={}", currentFrame.pts(), currentFrame.avFrame()->nb_samples);
  audioBuffer.size = av_samples_get_buffer_size(NULL, currentFrame.avFrame()->channels,
                                                currentFrame.avFrame()->nb_samples,
                                                (AVSampleFormat) currentFrame.avFrame()->format, 1);
  audioBuffer.data = currentFrame.avFrame()->data[0];
  audioBuffer.index = 0;
  audioBuffer.pts = currentFrame.pts();
}

void vivictpp::sdl::AudioOutput::start() {
  SDL_PauseAudioDevice(audioDevice, 0);
}

void vivictpp::sdl::AudioOutput::stop() {
  SDL_PauseAudioDevice(audioDevice, 1);
}
