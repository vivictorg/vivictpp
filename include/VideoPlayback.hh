// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIVICTPP_VIDEOPLAYBACK_HH_
#define VIVICTPP_VIDEOPLAYBACK_HH_

#include "VivictPPConfig.hh"
#include "time/Time.hh"
#include "time/TimeUtils.hh"
#include <cstdint>

#include "VideoInputs.hh"

namespace vivictpp {

//enum class PlaybackState { STOPPED, PLAYING, SEEKING };


struct PlaybackState {
  vivictpp::time::Time duration{0};
  vivictpp::time::Time pts{0};
  bool playing{false};
  bool seeking{false};
  bool ready{false};
  bool hasLeftSource{false};
  bool hasRightSource{false};
  int speedAdjust{0};
  int speedDen{1};
  int speedNum{1};
};

class VideoPlayback {
private:

  class SeekState {
  public:
    vivictpp::time::Time seekTarget;
    vivictpp::time::Time seekEndPos;
    bool seekDone{true};
    bool error;
  private:
    int currentSeekId{1}; // Used to ignore obsolete callbacks from previos seek operations
    std::mutex m;
  public:
    int seekStart(vivictpp::time::Time seekTarget);
    void seekFinished(int seekId, vivictpp::time::Time pos, bool err);
    void sync();
  };

  VideoInputs videoInputs;
  SeekState seekState;
  vivictpp::time::Time playbackStartPts{0};
  bool stepped{false};
  vivictpp::time::Time frameDuration;
  int64_t t0 = 0;
  PlaybackState playbackState;
  int seekRetry{0};
  vivictpp::logging::Logger logger;
private:
  void initPlaybackState();
public:
  VideoPlayback(VivictPPConfig vivictPPConfig);
  void setLeftSource(std::string source);
  void setRightSource(std::string source);
  void togglePlaying();
  void play();
  void pause();
  void seek(vivictpp::time::Time seekPts, vivictpp::time::Time streamSeekOffset = 0);
  void seekRelative(vivictpp::time::Time deltaPts);
  void seekRelativeFrame(int distance);
  bool checkAdvanceFrame(int64_t nextPresent);
  void advanceFrame(vivictpp::time::Time nextPts);
  VideoInputs &getVideoInputs() { return videoInputs; };
  bool isPlaying() { return playbackState.playing; }
  bool isSeeking() { return playbackState.seeking; }
  int adjustPlaybackSpeed(int delta) {
    playbackState.speedAdjust += delta;
    t0 = vivictpp::time::relativeTimeMicros();
    playbackStartPts = playbackState.pts;
    return playbackState.speedAdjust;
  }
  int increaseLeftFrameOffset() {
    int value = videoInputs.increaseLeftFrameOffset();
    if (!playbackState.playing) {
      advanceFrame(playbackState.pts);
      stepped = true;
    }
    return value;
  }
  int deccreaseLeftFrameOffset() {
    int value = videoInputs.decreaseLeftFrameOffset();
    if (!playbackState.playing) {
      advanceFrame(playbackState.pts);
      stepped = true;
    }
    return value;
  }
  const PlaybackState& getPlaybackState() { return playbackState; }

};

}  // namespace vivictpp


#endif /* VIVICTPP_VIDEOPLAYBACK_HH_ */
