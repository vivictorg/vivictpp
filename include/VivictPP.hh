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
#include "time/TimeUtils.hh"
#include "VideoInputs.hh"
#include "EventListener.hh"
#include "sdl/SDLEventLoop.hh"
#include "AVSync.hh"
#include "VivictPPConfig.hh"
#include "logging/Logging.hh"
#include "audio/AudioOutput.hh"
#include "EventLoop.hh"
#include "time/Time.hh"


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
  vivictpp::time::Time pts{0};
  vivictpp::time::Time nextPts{0};
  uint64_t lastFrameAdvance{std::numeric_limits<uint64_t>::min()}; // micros from monotonic clock
  int stepFrame{0};
  bool quit{false};
  int leftVideoStreamIndex{0};
  bool updateVideoMetadata{false};
  vivictpp::AVSync avSync;
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
           std::shared_ptr<EventScheduler> eventScheduler,
           vivictpp::audio::AudioOutputFactory &audioOutputFactory);
  virtual ~VivictPP() = default;
  void advanceFrame();
  PlaybackState togglePlaying();
  void audioSeek(vivictpp::time::Time pts);
  void seek(vivictpp::time::Time pts);
  void seekRelative(vivictpp::time::Time deltaT);
  void seekNextFrame();
  void seekPreviousFrame();
  void seekFrame(int delta);
  void switchStream(int delta);
  int nextFrameDelay();
  vivictpp::time::Time getPts() { return state.pts; }
  void onQuit();
  VideoInputs& getVideoInputs() { return videoInputs; }
  vivictpp::AVSync &getAVSync() { return state.avSync; }
  const PlayerState &getPlayerState() { return state; }
  const PlaybackState &getPlaybackState() { return state.playbackState; }
  bool isPlaying() { return state.playbackState == PlaybackState::PLAYING; }
  void queueAudio();

 private:
  PlayerState state;
  std::shared_ptr<EventScheduler> eventScheduler;
  VideoInputs videoInputs;
  std::shared_ptr<vivictpp::audio::AudioOutput> audioOutput;
  vivictpp::time::Time frameDuration;
  vivictpp::logging::Logger logger;
};

#endif  // VIVICTPP_H_
