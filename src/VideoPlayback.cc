// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "VideoPlayback.hh"
#include "time/Time.hh"

int vivictpp::VideoPlayback::SeekState::seekStart(
    vivictpp::time::Time seekTarget) {
  std::lock_guard<std::mutex> lg(m);
  currentSeekId++;
  seekDone = false;
  error = false;
  this->seekTarget = seekTarget;
  return currentSeekId;
};

void vivictpp::VideoPlayback::SeekState::seekFinished(int seekId,
                                                      vivictpp::time::Time pos,
                                                      bool err) {
  spdlog::info("Seek finished, pos={}, err={}", pos, err);
  std::lock_guard<std::mutex> lg(m);
  if (seekId != currentSeekId) {
    return;
  }
  seekDone = true;
  seekEndPos = pos;
  error = err;
}

void vivictpp::VideoPlayback::SeekState::sync() {
  std::lock_guard<std::mutex> lg(m);
}

vivictpp::VideoPlayback::VideoPlayback(vivictpp::ErrorQueue &errorQueue)
    : errorQueue(errorQueue), videoInputs(errorQueue),
      logger(vivictpp::logging::getOrCreateLogger("vivictpp::VideoPlayback")) {}

void vivictpp::VideoPlayback::initPlaybackState() {
  frameDuration = videoInputs.frameDuration();
  playbackState.pts = videoInputs.startTime();
  playbackState.duration = videoInputs.duration();
  playbackState.hasLeftSource = videoInputs.hasLeftSource();
  playbackState.hasRightSource = videoInputs.hasRightSource();
  playbackState.ready = playbackState.hasLeftSource;
}

void vivictpp::VideoPlayback::setLeftSource(const SourceConfig &source) {
  videoInputs.openLeft(source);
  initPlaybackState();
  seek(videoInputs.startTime());
}

void vivictpp::VideoPlayback::setRightSource(const SourceConfig &source) {
  videoInputs.openRight(source);
  initPlaybackState();
  seek(videoInputs.startTime());
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

void vivictpp::VideoPlayback::seek(vivictpp::time::Time seekPts,
                                   vivictpp::time::Time streamSeekOffset) {
  logger->debug("seek: pts={}", seekPts);

  // Cancel A-B loop if seeking outside loop boundaries
  if (playbackState.abLoopState == 2 &&
      !vivictpp::time::isNoPts(playbackState.loopPointA) &&
      !vivictpp::time::isNoPts(playbackState.loopPointB)) {
    if (seekPts < playbackState.loopPointA ||
        seekPts > playbackState.loopPointB) {
      logger->info("A-B Loop: Seeking outside loop boundaries, cancelling loop");
      playbackState.abLoopState = 0;
      playbackState.loopPointA = vivictpp::time::NO_TIME;
      playbackState.loopPointB = vivictpp::time::NO_TIME;
    }
  }

  seekPts = std::max(seekPts, videoInputs.minPts());
  if (videoInputs.hasMaxPts()) {
    seekPts = std::min(seekPts, videoInputs.maxPts());
  }
  if (!playbackState.seeking && videoInputs.ptsInRange(seekPts)) {
    logger->debug("seek: pts is in range");
    advanceFrame(seekPts);
    stepped = true;
    if (playbackState.playing) {
      playbackStartPts = seekPts;
      t0 = vivictpp::time::relativeTimeMicros();
    }
    // TODO: Make make special method for this
    int seekId = seekState.seekStart(seekPts);
    seekState.seekFinished(seekId, seekPts, false);
  } else {
    logger->debug("seek: pts is not in range");
    playbackState.seeking = true;
    int seekId = seekState.seekStart(seekPts);
    videoInputs.seek(
        seekPts,
        [this, seekId](vivictpp::time::Time pos, bool error) {
          this->seekState.seekFinished(seekId, pos, error);
        },
        streamSeekOffset);
  }
}

void vivictpp::VideoPlayback::seekRelative(vivictpp::time::Time deltaPts) {
  if (playbackState.seeking) {
    seekState.sync();
    seek(seekState.seekTarget + deltaPts);
  } else {
    seek(playbackState.pts + deltaPts);
  }
}

void vivictpp::VideoPlayback::seekRelativeFrame(int distance) {
  if (distance == 0)
    return;
  if (playbackState.seeking) {
    seek(seekState.seekTarget + distance * frameDuration);
  } else {
    vivictpp::time::Time seekPts;
    if (distance == 1)
      seekPts = videoInputs.nextPts();
    else if (distance == -1)
      seekPts = videoInputs.previousPts();
    else
      seekPts = playbackState.pts + distance * frameDuration;
    if (vivictpp::time::isNoPts(seekPts)) {
      seekPts = playbackState.pts + distance * frameDuration;
    }
    logger->debug("seekRelativeFrame  seeking to {}", seekPts);
    seek(seekPts);
  }
}

bool vivictpp::VideoPlayback::checkAdvanceFrame(int64_t nextPresent) {
  logger->debug("checkAdvanceFrame");
  if (playbackState.seeking) {
    seekState.sync();
    if (!seekState.seekDone) {
      logger->debug("checkAdvanceFrame: seekState.seekDone=false");
      return false;
    }
    //    logger->debug("checkAdvanceFrame playbackState.seeking=true,
    //    seekEndPos={}", seekState.seekEndPos);
    logger->debug("seekEndPos={} seekTarget={}", seekState.seekEndPos,
                  seekState.seekTarget);
    if (!videoInputs.ptsInRange(seekState.seekTarget) &&
        seekState.seekEndPos - seekState.seekTarget > 1000) {
      // In some circumstances, for instance if steeping back one frame from an
      // iframe Seeking may not work due to av_seek_frame apperantly seeking on
      // packet dts Which may be slightly lower than seek dts we calculate.
      // Therefore, retry seek with seeking to an earlier position in stream
      if (seekRetry == 0) {
        seekRetry = 1;
        seek(seekState.seekTarget, vivictpp::time::seconds(-1));
        return false;
      }
    }
    seekRetry = 0;
    advanceFrame(seekState.seekEndPos);
    playbackState.seeking = false;
    if (playbackState.playing) {
      playbackStartPts = seekState.seekEndPos;
      t0 = vivictpp::time::relativeTimeMicros();
    }
    return true;
  }
  if (!playbackState.playing) {
    if (stepped) {
      stepped = false;
      return true;
    }
    return false;
  }

  int speedFactorDen(1), speedFactorNum(1);
  if (playbackState.speedAdjust != 0) {
    if (playbackState.speedAdjust > 0) {
      // 99 / 70 is an aproximation of square root of 2
      speedFactorDen <<= (playbackState.speedAdjust / 2);
      if (playbackState.speedAdjust % 2) {
        speedFactorDen *= 99;
        speedFactorNum = 70;
      }
    } else {
      speedFactorNum <<= (-1 * playbackState.speedAdjust / 2);
      if ((-1 * playbackState.speedAdjust) % 2) {
        speedFactorNum *= 99;
        speedFactorDen = 70;
      }
    }
  }

  vivictpp::time::Time nextPts = videoInputs.nextPts();
  vivictpp::time::Time nextDisplayPts =
      playbackStartPts + speedFactorDen * (nextPresent - t0) / speedFactorNum;
  if (nextDisplayPts > videoInputs.maxPts()) {
    nextDisplayPts = videoInputs.maxPts();
  }
  if (videoInputs.ptsInRange(nextPts)) {
    if (nextDisplayPts >= nextPts) {
      while (nextDisplayPts >= nextPts && videoInputs.ptsInRange(nextPts)) {
        advanceFrame(nextPts);
        if (std::abs(nextPts - videoInputs.maxPts()) < 1000) {
          pause();
        }
        nextPts = videoInputs.nextPts();
      }
      return true;
    } else {
      return false;
    }
  }
  videoInputs.dropIfFullAndNextOutOfRange(nextPts, 1);
  return false;
};

void vivictpp::VideoPlayback::advanceFrame(vivictpp::time::Time nextPts) {
  logger->debug("advanceFrame nextPts={}", nextPts);
  playbackState.pts = nextPts;

  videoInputs.step(playbackState.pts);
  //  logger->debug("After advance frame pts={}", videoInputs.);
}

void vivictpp::VideoPlayback::cycleABLoop() {
  if (playbackState.abLoopState == 0) {
    // Set point A to current position
    playbackState.loopPointA = playbackState.pts;
    playbackState.abLoopState = 1;
    logger->info("A-B Loop: Point A set at {}", playbackState.loopPointA);
  } else if (playbackState.abLoopState == 1) {
    // Set point B to current position and activate loop
    playbackState.loopPointB = playbackState.pts;
    if (playbackState.loopPointB > playbackState.loopPointA) {
      playbackState.abLoopState = 2;
      logger->info("A-B Loop: Point B set at {}, loop activated",
                   playbackState.loopPointB);
    } else {
      // B must be after A, reset to state 0
      playbackState.abLoopState = 0;
      playbackState.loopPointA = vivictpp::time::NO_TIME;
      playbackState.loopPointB = vivictpp::time::NO_TIME;
      logger->info("A-B Loop: Point B must be after point A, loop cancelled");
    }
  } else {
    // Cancel loop
    playbackState.abLoopState = 0;
    playbackState.loopPointA = vivictpp::time::NO_TIME;
    playbackState.loopPointB = vivictpp::time::NO_TIME;
    logger->info("A-B Loop: Cancelled");
  }
}

bool vivictpp::VideoPlayback::checkABLoop() {
  if (playbackState.abLoopState == 2 &&
      !vivictpp::time::isNoPts(playbackState.loopPointA) &&
      !vivictpp::time::isNoPts(playbackState.loopPointB)) {
    if (playbackState.pts >= playbackState.loopPointB) {
      logger->debug("A-B Loop: Reached point B, seeking back to point A");
      seek(playbackState.loopPointA);
      return true;
    }
  }
  return false;
}
