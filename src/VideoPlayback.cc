// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "VideoPlayback.hh"
#include "time/Time.hh"

int vivictpp::VideoPlayback::SeekState::seekStart(vivictpp::time::Time seekTarget) {
  std::lock_guard<std::mutex> lg(m);
  currentSeekId++;
  seekDone = false;
  error = false;
  this->seekTarget = seekTarget;
  return currentSeekId;
};

void vivictpp::VideoPlayback::SeekState::seekFinished(int seekId, vivictpp::time::Time pos, bool err) {
  spdlog::debug("Seek finished");
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

vivictpp::VideoPlayback::VideoPlayback(const std::vector<SourceConfig> &sourceConfigs):
  videoInputs(),
  logger(vivictpp::logging::getOrCreateLogger("vivictpp::VideoPlayback"))
{
  if (sourceConfigs.size() >= 1) {
    videoInputs.openLeft(sourceConfigs[0]);
  }
  if (sourceConfigs.size() >=2 ) {
    videoInputs.openRight(sourceConfigs[1]);
  }
  if (!sourceConfigs.empty()) {
    initPlaybackState();
  }
}

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

void vivictpp::VideoPlayback::seek(vivictpp::time::Time seekPts,  vivictpp::time::Time streamSeekOffset) {
  logger->debug("seek: pts={}", seekPts);
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
    videoInputs.seek(seekPts, [this, seekId](vivictpp::time::Time pos, bool error) {
      this->seekState.seekFinished(seekId, pos, error);
    }, streamSeekOffset);
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
  if (distance == 0) return;
  if (playbackState.seeking) {
    seek(seekState.seekTarget + distance * frameDuration);
  } else {
    vivictpp::time::Time seekPts;
    if (distance == 1) seekPts = videoInputs.nextPts();
    else if (distance == -1) seekPts = videoInputs.previousPts();
    else seekPts = playbackState.pts + distance * frameDuration;
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
//    logger->debug("checkAdvanceFrame playbackState.seeking=true, seekEndPos={}", seekState.seekEndPos);
    if (!videoInputs.ptsInRange(seekState.seekTarget) && seekState.seekEndPos - seekState.seekTarget > 1000) {
      logger->debug("seekEndPos={} seekTarget={}", seekState.seekEndPos, seekState.seekTarget);
      // In some circumstances, for instance if steeping back one frame from an iframe
      // Seeking may not work due to av_seek_frame apperantly seeking on packet dts
      // Which may be slightly lower than seek dts we calculate. Therefore, retry seek
      // with seeking to an earlier position in stream
      if (seekRetry == 0) {
        seekRetry = 1;
        seek(seekState.seekTarget,  vivictpp::time::seconds(-1));
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
  vivictpp::time::Time nextDisplayPts = playbackStartPts + speedFactorDen *
                                        (nextPresent - t0) / speedFactorNum;
  if (nextDisplayPts > videoInputs.maxPts()) {
      nextDisplayPts = videoInputs.maxPts();
  }
  if (videoInputs.ptsInRange(nextPts)) {
    if (nextDisplayPts >= nextPts) {
      while( nextDisplayPts >= nextPts && videoInputs.ptsInRange(nextPts)) {
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
