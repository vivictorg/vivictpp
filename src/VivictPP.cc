// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "VivictPP.hh"

#include "logging/Logging.hh"
#include "time/Time.hh"
#include <cstdint>
#include <utility>


std::string playbackStateName(PlaybackState playbackState) {
  switch (playbackState) {
  case PlaybackState::PLAYING:
    return "PLAYING";
  case PlaybackState::STOPPED:
    return "STOPPED";
  case PlaybackState::SEEKING:
    return "SEEKING";
  }
  return "";
}

PlaybackState PlayerState::togglePlaying() {
    spdlog::debug("Current playbackState: {}",
                  playbackStateName(playbackState));
    switch (playbackState) {
    case PlaybackState::PLAYING:
      playbackState = PlaybackState::STOPPED;
      break;
    case PlaybackState::STOPPED:
      playbackState = PlaybackState::PLAYING;
      break;
    case PlaybackState::SEEKING:
      break;
    }
    spdlog::debug("New playbackState: {}", playbackStateName(playbackState));
    return playbackState;
  }

VivictPP::VivictPP(VivictPPConfig vivictPPConfig,
                   std::shared_ptr<EventScheduler> eventScheduler,
                   vivictpp::audio::AudioOutputFactory &audioOutputFactory)
  : state(),
    eventScheduler(eventScheduler),
    videoInputs(),
    audioOutput(nullptr),
    logger(vivictpp::logging::getOrCreateLogger("VivictPP")),
    seeklog(vivictpp::logging::getOrCreateLogger("seeklog")){
  if (!vivictPPConfig.disableAudio && videoInputs.hasAudio()) {
    audioOutput = audioOutputFactory.create(videoInputs.getAudioCodecContext());
  }

  auto metadata = videoInputs.metadata();
  state.pts = videoInputs.minPts();
  state.nextPts = state.pts;
  if (vivictPPConfig.sourceConfigs.size() == 1) {
    frameDuration = metadata[0][0].frameDuration;
  } else {
    frameDuration =
      std::min(metadata[0][0].frameDuration, metadata[1][0].frameDuration);
  }
}

int VivictPP::nextFrameDelay() {

  int64_t videoDiff = state.avSync.diffMicros(state.pts);
  int64_t clockPts = state.avSync.clock();

  int corr = vivictpp::time::toMillis(videoDiff);
  if (corr > 30) corr = 30;
  if (corr < -30) corr = -30;
  int speedFactorDen(1), speedFactorNum(1);
  if (state.playbackSpeed != 0) {
    corr = 0;
    if (state.playbackSpeed > 0) {
      // 99 / 70 is an aproximation of square root of 2
      speedFactorDen <<= (state.playbackSpeed / 2);
      if (state.playbackSpeed % 2) {
        speedFactorDen *= 99;
        speedFactorNum = 70;
      }
    } else {
      speedFactorNum <<= (-1 * state.playbackSpeed / 2);
      if ((-1 * state.playbackSpeed) % 2) {
        speedFactorNum *= 99;
        speedFactorDen = 70;
      }
    }
  }
  int delay = std::max(5, (int)(av_rescale(state.nextPts - state.pts, speedFactorDen * 1000, speedFactorNum * vivictpp::time::TIME_BASE) + corr));
  logger->debug("VivictPP::nextFrameDelay videoPts={} clockPts={} videoDelta={}ms corr = {}ms, delay = {}ms",
               state.pts, clockPts / 1e6, videoDiff/1e6, corr, delay);
  return delay;
}

int VivictPP::adjustPlaybackSpeed(int delta) {
  //state.playbackSpeed = std::max(0, state.playbackSpeed + delta);
  state.playbackSpeed += delta;
  if (state.playbackSpeed == 0) {
    state.avSync.playbackStart(state.pts);
  }
  return state.playbackSpeed;
}

void VivictPP::advanceFrame() {
  logger->trace("VivictPP::advanceFrame pts={} nextPts={}", state.pts,
                state.nextPts);
  if (vivictpp::time::isNoPts(state.nextPts)) {
    state.nextPts = videoInputs.nextPts();
    if (!vivictpp::time::isNoPts(state.nextPts)) {
      eventScheduler->scheduleAdvanceFrame(nextFrameDelay());
      return;
    }
  }
  if (videoInputs.ptsInRange(state.nextPts) && (!audioOutput || videoInputs.audioFrames().ptsInRange(state.nextPts))) {
    logger->trace("VivictPP::advanceFrame nextPts is in range {}",
                  state.nextPts);
    if (state.seeking) {
      audioSeek(state.nextPts);
    }

    if (state.nextPts > state.pts || state.seeking) {
      videoInputs.stepForward(state.nextPts);
    } else {
      videoInputs.stepBackward(state.nextPts);
    }
    state.pts = state.nextPts;
    bool wasSeeking = state.seeking;
    state.seeking = false;
    if (state.playbackState == PlaybackState::PLAYING) {
      if (wasSeeking) {
        state.avSync.playbackStart(state.pts);
      }
      if (videoInputs.hasMaxPts() && state.pts >= videoInputs.maxPts()) {
        togglePlaying();
      } else {
        state.nextPts = videoInputs.nextPts();
        logger->trace("VivictPP::advanceFrame nextPts={}", state.nextPts);
        if (vivictpp::time::isNoPts(state.nextPts)) {
          eventScheduler->scheduleAdvanceFrame(5);
        } else {
          eventScheduler->scheduleAdvanceFrame(nextFrameDelay());
        }
      }
    }
    state.lastFrameAdvance = vivictpp::time::relativeTimeMicros();
    eventScheduler->scheduleRefreshDisplay(0);
  } else {
    logger->trace("VivictPP::advanceFrame nextPts is out of range {}", state.nextPts);
    videoInputs.dropIfFullAndNextOutOfRange(state.pts, state.seeking ? 0 : 1);
    eventScheduler->scheduleAdvanceFrame(5);
  }
}

void VivictPP::queueAudio() {
  if (!audioOutput) {
    return;
  }
  int delay = 0;
  vivictpp::time::Time queueDuration = audioOutput->queueDuration();
  logger->debug("vivictPP::queueAudio queueDuration={} audioOutput->currentPts={}",
               queueDuration, audioOutput->currentPts());
  if (state.playbackState != PlaybackState::PLAYING) {
    return;
  }
  if (queueDuration > 200000) {
    delay = std::max(1, (int) ((queueDuration - 40000) / 1000));
  } else {
    vivictpp::time::Time nextPts = videoInputs.audioFrames().nextPts();
    if (!vivictpp::time::isNoPts(nextPts)) {
      int c = 0;
      while (!vivictpp::time::isNoPts(nextPts) && c < 5) {
        vivictpp::time::Time prevPts = videoInputs.audioFrames().currentPts();
        videoInputs.audioFrames().stepForward(nextPts);
        queueDuration += (nextPts - prevPts);
        audioOutput->queueAudio(videoInputs.audioFrames().first());
        nextPts = videoInputs.audioFrames().nextPts();
        c++;
      }
      delay = std::max(1, (int) ((queueDuration - 40000) / 1000));
      logger->debug("vivictpp::ui::VivictUI::queueAudio framesQueued={} delay={}", c, delay);
    } else {
      delay = 10;
    }
  }
  eventScheduler->scheduleQueueAudio(delay);
}

PlaybackState VivictPP::togglePlaying() {
  if (state.togglePlaying() == PlaybackState::PLAYING) {
    audioSeek(state.pts);
    queueAudio();
    if (audioOutput) {
      audioOutput->start();
    }
    state.avSync.playbackStart(state.pts);
    /*
    if (state.nextPts == state.pts) {
      state.nextPts = state.pts + frameDuration;
    }
    */
    state.nextPts = state.pts;
    eventScheduler->scheduleAdvanceFrame(0);
  } else {
    if (audioOutput) {
      audioOutput->stop();
    }
    eventScheduler->clearAdvanceFrame();
    eventScheduler->scheduleRefreshDisplay(0);
  }
  return state.playbackState;
}

void VivictPP::seekPreviousFrame() {
  if (state.seeking) {
    seek(state.nextPts - frameDuration);
  } else {
    vivictpp::time::Time previousPts = videoInputs.previousPts();
    if (vivictpp::time::isNoPts(previousPts)) {
      previousPts = state.pts - frameDuration;
    }
    seek(previousPts);
  }
}

void VivictPP::seekNextFrame() {
  if (state.seeking) {
    seek(state.nextPts + frameDuration);
  } else {
    vivictpp::time::Time nextPts = videoInputs.nextPts();
    if (vivictpp::time::isNoPts(nextPts)) {
      nextPts = state.pts + frameDuration;
    }
    seek(nextPts);
  }
}

void VivictPP::seekRelative(vivictpp::time::Time deltaT) {
  seeklog->debug("VivictPP::seekRelative deltaT={} pts={} nextPts={} seeking={}", deltaT, state.pts, state.nextPts, state.seeking);
  if (state.seeking) {
    seek(state.nextPts + deltaT);
  } else {
    seek(state.pts + deltaT);
  }
}

void VivictPP::seek(vivictpp::time::Time nextPts) {
  nextPts = std::max(nextPts, videoInputs.minPts());
  if (videoInputs.hasMaxPts()) {
    nextPts = std::min(nextPts, videoInputs.maxPts());
  }
  state.nextPts = nextPts;
  seeklog->debug("VivictPP::seek pts={} nextPts={} seeking={}", state.pts, state.nextPts, state.seeking);
  if (videoInputs.ptsInRange(state.nextPts) &&
      (!audioOutput || videoInputs.audioFrames().ptsInRange(state.nextPts))) {
    seeklog->debug("VivictPP::seek Seek pts in range");
    if (state.playbackState == PlaybackState::PLAYING) {
      togglePlaying();
      audioSeek(state.nextPts);
      togglePlaying();
    } else {
      audioSeek(state.nextPts);
      eventScheduler->clearAdvanceFrame();
      eventScheduler->scheduleAdvanceFrame(5);
    }
  } else {
    seeklog->debug("VivictPP::seek Seek pts not in range");
    state.seeking = true;
    videoInputs.seek(state.nextPts, [this](vivictpp::time::Time pos, bool error) {
      this->eventScheduler->scheduleSeekFinished(pos, error);
    });
    eventScheduler->clearAdvanceFrame();
//    eventScheduler->scheduleAdvanceFrame(5);
  }
}

void VivictPP::onSeekFinished(vivictpp::time::Time seekedPos, bool error) {
  this->seeklog->debug("Seek callback called with pos={}, error={}", seekedPos, error);
  if (error) {
    state.nextPts = state.pts;
  } else {
    state.nextPts = seekedPos;
  }
  eventScheduler->scheduleAdvanceFrame(0);
}

void VivictPP::audioSeek(vivictpp::time::Time pts) {
  if (!audioOutput) {
    return;
  }
  audioOutput->clearQueue();
  if (videoInputs.audioFrames().nextPts() < pts) {
    videoInputs.audioFrames().stepForward(pts);
  } else {
    videoInputs.audioFrames().stepBackward(pts);
  }
}

void VivictPP::seekFrame(int delta) { state.stepFrame = delta; }

void VivictPP::onQuit() {
  if (audioOutput) {
    audioOutput->stop();
  }
}

void VivictPP::switchStream(int delta) {
  logger->debug("VivictPP::switchStream delta={}", delta);
  /*
  int newIndex = (state.leftVideoStreamIndex + delta) % videoInputs.metadata()[0].size();
//  if (state.leftVideoStreamIndex + delta >= 0) {
  state.leftVideoStreamIndex = newIndex;
    screenOutput.setLeftMetadata(
                                 videoInputs.metadata()[0][state.leftVideoStreamIndex]);
    if (!splitScreenDisabled) {
      // screenOutput.setRightMetadata(metadata[1][0]);
    }
    videoInputs.selectVideoStreamLeft(state.leftVideoStreamIndex);
    state.seeking = true;
    eventScheduler->scheduleAdvanceFrame(5);
//    eventScheduler->scheduleRefreshDisplay(0);
//  }
*/
}
