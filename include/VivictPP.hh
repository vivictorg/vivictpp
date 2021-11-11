// SPDX-FileCopyrightText: 2020 Sveriges Television AB
// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIVICTPP_H_
#define VIVICTPP_H_

#include <string>

#include "ui/DisplayState.hh"
#include "workers/FrameBuffer.hh"
#include "ui/ScreenOutput.hh"
#include "TimeUtils.hh"
#include "VideoInputs.hh"
#include "EventListener.hh"
#include "sdl/SDLEventLoop.hh"
#include "AVSync.hh"
#include "VivictPPConfig.hh"
#include "logging/Logging.hh"
#include "audio/AudioOutput.hh"
#include "EventLoop.hh"



extern "C" {
#include <SDL.h>
#include <SDL_thread.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <unistd.h>
}

enum class PlaybackState { STOPPED, PLAYING, SEEKING };

std::string playbackStateName(PlaybackState playbackState);

struct PlayerState {
  PlaybackState playbackState{PlaybackState::STOPPED};
  bool seeking{false};
  double pts{0};
  double nextPts{0};
  int stepFrame{0};
  bool quit{false};
  int leftVideoStreamIndex{0};
  bool updateVideoMetadata{false};
  AVSync avSync;
  PlaybackState togglePlaying();
};
/*
struct AudioFrames {
  std::vector<vivictpp::libav::Frame>
};
*/


class VivictPP  {
public:
  VivictPP(VivictPPConfig vivictPPConfig,
           EventScheduler &eventScheduler,
           vivictpp::audio::AudioOutputFactory &audioOutputFactory);
  virtual ~VivictPP() = default;
  void advanceFrame();
  PlaybackState togglePlaying();
  void audioSeek(double pts);
  void seek(double pts);
  void seekRelative(double deltaT);
  void seekNextFrame();
  void seekPreviousFrame();
  void seekFrame(int delta);
  void switchStream(int delta);
  int nextFrameDelay();
  double getPts() { return state.pts; }
  void onQuit();
  VideoInputs& getVideoInputs() { return videoInputs; }
  AVSync &getAVSync() { return state.avSync; }
  const PlaybackState &getPlaybackState() { return state.playbackState; }
  void queueAudio();

 private:
//vivictpp::sdl::SDLEventLoop eventLoop;
  PlayerState state;
  EventScheduler &eventScheduler;
  const AVPixelFormat pixelFormat;
  VideoInputs videoInputs;
//  vivictpp::ui::ScreenOutput screenOutput;
  std::shared_ptr<vivictpp::audio::AudioOutput> audioOutput;
  bool splitScreenDisabled;
  double frameDuration;
  vivictpp::logging::Logger logger;
};

#endif  // VIVICTPP_H_
