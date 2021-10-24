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
#include "sdl/AudioOutput.hh"
#include "AVSync.hh"
#include "VivictPPConfig.hh"
#include "logging/Logging.hh"



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
    vivictpp::ui::DisplayState displayState;
  bool quit{false};
  int leftVideoStreamIndex{0};
  bool updateVideoMetadata{false};
  AVSync avSync;
  PlaybackState togglePlaying();
};

class VivictPP : EventListener {
public:
  VivictPP(VivictPPConfig vivictPPConfig);
  virtual ~VivictPP() = default;
  int run();
  void mouseDragStart() override;
  void mouseDragEnd() override;
  void mouseDrag(int xrel, int yrel) override;
  void mouseMotion(int x, int y) override;
  void mouseWheel(int x, int y) override;
  void mouseClick(int x, int y) override;
  void keyPressed(std::string key) override;
  void advanceFrame() override;
  void refreshDisplay() override;
  void queueAudio() override;

private:
  void togglePlaying();
  void audioSeek(double pts);
  void seek(double pts);
  void seekRelative(double deltaT);
  void seekNextFrame();
  void seekPreviousFrame();
  void seekFrame(int delta);
  void switchStream(int delta);
  int nextFrameDelay();
  void onQuit();

 private:
  vivictpp::sdl::SDLEventLoop eventLoop;
  PlayerState state;
  const AVPixelFormat pixelFormat;
  VideoInputs videoInputs;
  vivictpp::ui::ScreenOutput screenOutput;
  std::shared_ptr<vivictpp::sdl::AudioOutput> audioOutput;
  bool splitScreenDisabled;
  double frameDuration;
  vivictpp::logging::Logger logger;
};

#endif  // VIVICTPP_H_
