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
  frameDuration(vivictPPConfig.sourceConfigs.size() == 1 ?
                videoInputs.metadata()[0][0].frameDuration :
                std::min(videoInputs.metadata()[0][0].frameDuration,
                         videoInputs.metadata()[1][0].frameDuration)),
  logger(vivictpp::logging::getOrCreateLogger("vivictpp::VideoPlayback"))
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

void vivictpp::VideoPlayback::seek(vivictpp::time::Time seekPts,  vivictpp::time::Time streamSeekOffset) {
  seekPts = std::max(seekPts, videoInputs.minPts());
  if (videoInputs.hasMaxPts()) {
    seekPts = std::min(seekPts, videoInputs.maxPts());
  }
  if (!playbackState.seeking && videoInputs.ptsInRange(seekPts)) {
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
    playbackState.seeking = true;
    int seekId = seekState.seekStart(seekPts);
    videoInputs.seek(seekPts, [this, seekId](vivictpp::time::Time pos, bool error) {
      this->seekState.seekFinished(seekId, pos, error);
    }, streamSeekOffset);
  }
}

void vivictpp::VideoPlayback::seekRelative(vivictpp::time::Time deltaPts) {
  if (playbackState.seeking) {
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
    logger->info("seekRelativeFrame  seeking to {}", seekPts);
    seek(seekPts);
  }
}

bool vivictpp::VideoPlayback::checkAdvanceFrame(int64_t nextPresent) {
  if (playbackState.seeking && !seekState.seekDone) {
    // videoInputs.dropIfFullAndNextOutOfRange(nextPts, 1);
    return false;
  }
  if (playbackState.seeking) {
//    logger->info("checkAdvanceFrame playbackState.seeking=true, seekEndPos={}", seekState.seekEndPos);
    if (seekState.seekEndPos - seekState.seekTarget > 1000) {
      logger->info("seekEndPos={} seekTarget={}", seekState.seekEndPos, seekState.seekTarget);
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
  if (videoInputs.ptsInRange(nextPts)) {
    if (nextDisplayPts >= nextPts) {
      while( nextDisplayPts >= nextPts) {
        advanceFrame(nextPts);
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
  logger->info("advanceFrame nextPts={}", nextPts);
  playbackState.pts = nextPts;

  videoInputs.step(playbackState.pts);
//  logger->info("After advance frame pts={}", videoInputs.);
}
