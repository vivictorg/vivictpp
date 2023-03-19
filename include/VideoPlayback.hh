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
  };

  VideoInputs videoInputs;
  SeekState seekState;
  vivictpp::time::Time playbackStartPts{0};
  bool stepped{false};
  vivictpp::time::Time frameDuration;
  int64_t t0 = 0;
  PlaybackState playbackState;
  vivictpp::logging::Logger logger;

public:
  VideoPlayback(VivictPPConfig vivictPPConfig);
  void togglePlaying();
  void play();
  void pause();
  void seek(vivictpp::time::Time seekPts);
  void seekRelative(vivictpp::time::Time deltaPts);
  void seekRelativeFrame(int distance);
  bool checkAdvanceFrame(int64_t nextPresent);
  void advanceFrame(vivictpp::time::Time nextPts);
  VideoInputs &getVideoInputs() { return videoInputs; };
  bool isPlaying() { return playbackState.playing; }
  bool isSeeking() { return playbackState.seeking; }
  const PlaybackState& getPlaybackState() { return playbackState; }

};

}  // namespace vivictpp


#endif /* VIVICTPP_VIDEOPLAYBACK_HH_ */
