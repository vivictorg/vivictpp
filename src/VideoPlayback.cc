#include "VideoPlayback.hh"

int vivictpp::VideoPlayback::SeekState::seekStart() {
  std::lock_guard<std::mutex> lg(m);
  currentSeekId++;
  seekDone = false;
  error = false;
  return currentSeekId;
};

void vivictpp::VideoPlayback::SeekState::seekFinished(int seekId, vivictpp::time::Time pos, bool err) {
  std::lock_guard<std::mutex> lg(m);
  if (seekId != currentSeekId) {
    return;
  }
  seekDone = true;
  seekEndPos = pos;
  error = err;
}

vivictpp::VideoPlayback::VideoPlayback(VivictPPConfig vivictPPConfig):
  videoInputs(vivictPPConfig)
{
  playbackState.pts = videoInputs.startTime();
  playbackState.duration = videoInputs.duration();
}

void vivictpp::VideoPlayback::togglePlaying() {
  if (playbackState.seeking) {
    return;
  }
  if (!playbackState.playing) {
    play();
  } else {
    pause();
  }
}

void vivictpp::VideoPlayback::play() {
  t0 = vivictpp::time::relativeTimeMicros();
  playbackStartPts = playbackState.pts;
  playbackState.playing = true;
}

void vivictpp::VideoPlayback::pause() { playbackState.playing = false; }

void vivictpp::VideoPlayback::seek(vivictpp::time::Time seekPts) {
  seekPts = std::max(seekPts, videoInputs.minPts());
  if (videoInputs.hasMaxPts()) {
    seekPts = std::min(seekPts, videoInputs.maxPts());
  }
  if (!playbackState.seeking && videoInputs.ptsInRange(seekPts)) {
    advanceFrame(seekPts);
    if (playbackState.playing) {
      playbackStartPts = seekPts;
      t0 = vivictpp::time::relativeTimeMicros();
    }
  } else {
    playbackState.seeking = true;
    int seekId = seekState.seekStart();
    videoInputs.seek(seekPts, [this, seekId](vivictpp::time::Time pos, bool error) {
      this->seekState.seekFinished(seekId, pos, error);
    });
  }
}

bool vivictpp::VideoPlayback::checkdvanceFrame(int64_t nextPresent) {
  if (playbackState.seeking && !seekState.seekDone) {
    return false;
  }
  if (playbackState.seeking) {
    advanceFrame(seekState.seekEndPos);
    playbackState.seeking = false;
    if (playbackState.playing) {
      playbackStartPts = seekState.seekEndPos;
      t0 = vivictpp::time::relativeTimeMicros();
    }
    return true;
  }
  if (!playbackState.playing) {
    return false;
  }
  vivictpp::time::Time nextPts = videoInputs.nextPts();
  if (videoInputs.ptsInRange(nextPts)) {
    if ((nextPresent - t0) >= (nextPts - playbackStartPts)) {
      playbackState.pts = nextPts;
      videoInputs.stepForward(nextPts);
      return true;
    } else {
      return false;
    }
  }
  videoInputs.dropIfFullAndNextOutOfRange(nextPts, 1);
  return false;
};

void vivictpp::VideoPlayback::advanceFrame(vivictpp::time::Time nextPts) {
  bool forward = nextPts > playbackState.pts;
  playbackState.pts = nextPts;
  if (forward) {
    videoInputs.stepForward(playbackState.pts);
  } else {
    videoInputs.stepBackward(playbackState.pts);
  }
}
