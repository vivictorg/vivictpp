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
  videoInputs(vivictPPConfig),
  pts(videoInputs.startTime())
{ }

void vivictpp::VideoPlayback::togglePlaying() {
  if (seeking) {
    return;
  }
  if (!playing) {
    play();
  } else {
    pause();
  }
}

void vivictpp::VideoPlayback::play() {
  t0 = vivictpp::time::relativeTimeMicros();
  playbackStartPts = pts;
  playing = true;
}

void vivictpp::VideoPlayback::pause() { playing = false; }

void vivictpp::VideoPlayback::seek(vivictpp::time::Time seekPts) {
  seekPts = std::max(seekPts, videoInputs.minPts());
  if (videoInputs.hasMaxPts()) {
    seekPts = std::min(seekPts, videoInputs.maxPts());
  }
  if (!seeking && videoInputs.ptsInRange(seekPts)) {
    advanceFrame(seekPts);
    if (playing) {
      playbackStartPts = seekPts;
      t0 = vivictpp::time::relativeTimeMicros();
    }
  } else {
    seeking = true;
    int seekId = seekState.seekStart();
    videoInputs.seek(seekPts, [this, seekId](vivictpp::time::Time pos, bool error) {
      this->seekState.seekFinished(seekId, pos, error);
    });
  }
}

bool vivictpp::VideoPlayback::checkdvanceFrame(int64_t nextPresent) {
  if (seeking && !seekState.seekDone) {
    return false;
  }
  if (seeking) {
    advanceFrame(seekState.seekEndPos);
    seeking = false;
    if (playing) {
      playbackStartPts = seekState.seekEndPos;
      t0 = vivictpp::time::relativeTimeMicros();
    }
    return true;
  }
  if (!playing) {
    return false;
  }
  vivictpp::time::Time nextPts = videoInputs.nextPts();
  if (videoInputs.ptsInRange(nextPts)) {
    if ((nextPresent - t0) >= (nextPts - playbackStartPts)) {
      pts = nextPts;
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
  bool forward = nextPts > pts;
  pts = nextPts;
  if (forward) {
    videoInputs.stepForward(pts);
  } else {
    videoInputs.stepBackward(pts);
  }
}
