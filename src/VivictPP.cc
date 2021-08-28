// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "VivictPP.hh"

#include "logging/Logging.hh"
#include <bits/stdint-intn.h>
#include <libavcodec/packet.h>
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

VideoMetadata* metadataPtr(const std::vector<VideoMetadata> &v) {
  if (v.empty()) {
    return nullptr;
  }
  return new VideoMetadata(v[0]);
}

VivictPP::VivictPP(VivictPPConfig vivictPPConfig)
  : state(),
    pixelFormat(AV_PIX_FMT_YUV420P),
    videoInputs(vivictPPConfig),
    screenOutput(metadataPtr(videoInputs.metadata()[0]),
                 metadataPtr(videoInputs.metadata()[1]),
                 vivictPPConfig.sourceConfigs),
    splitScreenDisabled(vivictPPConfig.sourceConfigs.size() == 1),
    logger(vivictpp::logging::getOrCreateLogger("VivictPP")) {
  if (!vivictPPConfig.disableAudio && videoInputs.hasAudio()) {
    audioOutput.reset(new vivictpp::sdl::AudioOutput(videoInputs.getAudioCodecContext(), videoInputs.audioFrames(), state.avSync));
  }
  auto metadata = videoInputs.metadata();
  state.pts = metadata[0][0].startTime;
  state.nextPts = state.pts;
  state.displayState.splitScreenDisabled = splitScreenDisabled;
  if (splitScreenDisabled) {
    frameDuration =1.0d / metadata[0][0].frameRate;
  } else {
    frameDuration =
      1.0d / std::max(metadata[0][0].frameRate, metadata[1][0].frameRate);
  }
}

int VivictPP::nextFrameDelay() {

  int64_t videoDiff = state.avSync.diffMicros(vivictpp::util::toMicros(state.pts));
  int64_t clockPts = state.avSync.clock();

  int corr = vivictpp::util::toMillis(videoDiff);
  if (corr > 30) corr = 30;
  if (corr < -30) corr = -30;
  int delay = std::max(5, (int)((state.nextPts - state.pts) * 1000 + corr));
  logger->debug("VivictPP::nextFrameDelay videoPts={} clockPts={} audioOutput.currentPts()={} videoDelta={}ms corr = {}ms, delay = {}ms",
               state.pts, clockPts / 1e6, audioOutput ? audioOutput->currentPts() : 0, videoDiff/1e6, corr, delay);
  return delay;
}

void VivictPP::queueAudio() {
  if (!audioOutput) {
    return;
  }
  int delay = 0;
  double queueDuration = audioOutput->queueDuration();
  logger->debug("VivictPP::queueAudio queueDuration={} audioOutput->currentPts={}",
               queueDuration, audioOutput->currentPts());
  if (state.playbackState != PlaybackState::PLAYING) {
    return;
  }
  if (queueDuration > 0.2) {
    delay = std::max(1, (int) ((audioOutput->queueDuration() - 0.04) * 1000));
  } else {
    double nextPts = videoInputs.audioFrames().nextPts();
    if (!isnan(nextPts)) {
      int c = 0;
      while (!isnan(nextPts) && c < 5) {
        videoInputs.audioFrames().stepForward(nextPts);
        audioOutput->queueAudio(videoInputs.audioFrames().first());
        nextPts = videoInputs.audioFrames().nextPts();
        c++;
      }
      delay = std::max(1, (int) ((audioOutput->queueDuration() - 0.04) * 1000));
      logger->debug("VivictPP::queueAudio framesQueued={} delay={}", c, delay);
    } else {
      delay = 10;
    }
  }
  //  logger->info("VivictPP::queueAudio delay={}", delay);
  eventLoop.scheduleQueueAudio(delay);
}

void VivictPP::advanceFrame() {
  logger->trace("VivictPP::advanceFrame pts={} nextPts={}", state.pts,
                state.nextPts);
  if (isnan(state.nextPts)) {
    state.nextPts = videoInputs.nextPts();
    if (!isnan(state.nextPts)) {
      eventLoop.scheduleAdvanceFrame(nextFrameDelay());
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
    state.seeking = false;
    if (state.playbackState == PlaybackState::PLAYING) {
      state.nextPts = videoInputs.nextPts();
      logger->trace("VivictPP::advanceFrame nextPts={}", state.nextPts);
      if (isnan(state.nextPts)) {
        eventLoop.scheduleAdvanceFrame(5);
      } else {
        //      state.nextPts = state.pts + 0.04; // TODO: Use either pts from
        //      framebuffer or delta from framerate
        eventLoop.scheduleAdvanceFrame(nextFrameDelay());
      }
    }
    eventLoop.scheduleRefreshDisplay(0);
  } else {
    logger->trace("nextPts is out of range {}", state.nextPts);
    videoInputs.dropIfFullAndNextOutOfRange(state.pts, state.seeking ? 0 : 1);
    eventLoop.scheduleAdvanceFrame(5);
  }
}

void VivictPP::refreshDisplay() {
  logger->trace("VivictPP::refreshDisplay");
  std::array<vivictpp::libav::Frame, 2> frames = videoInputs.firstFrames();
  if (state.displayState.displayTime) {
    state.displayState.timeStr = vivictpp::util::formatTime(state.pts);
  }
  state.displayState.pts = state.pts;
  screenOutput.displayFrame(frames, state.displayState);
}

void VivictPP::togglePlaying() {
  if (state.togglePlaying() == PlaybackState::PLAYING) {
    state.displayState.isPlaying = true;
    queueAudio();
    if (audioOutput) {
      audioOutput->start();
    }
    state.avSync.playbackStart(vivictpp::util::toMicros(state.pts));
    if (state.nextPts == state.pts) {
      state.nextPts = state.pts + frameDuration;
    }
    eventLoop.scheduleAdvanceFrame(0);
  } else {
    state.displayState.isPlaying = false;
    if (audioOutput) {
      audioOutput->stop();
    }
  }
}

void VivictPP::seekPreviousFrame() {
  double previousPts = videoInputs.previousPts();
  if (isnan(previousPts)) {
    previousPts = state.pts - frameDuration;
  }
  seek(previousPts);
}

void VivictPP::seekNextFrame() {
  double nextPts = videoInputs.nextPts();
  if (isnan(nextPts)) {
    nextPts = state.pts + frameDuration;
  }
  seek(nextPts);
}

void VivictPP::seekRelative(double deltaT) {
  seek(state.pts + deltaT);
}

void VivictPP::seek(double nextPts) {
  state.nextPts = nextPts;
  logger->debug("VivictPP::seek pts={} nextPts={}", state.pts, state.nextPts);
  if (videoInputs.ptsInRange(state.nextPts) &&
      (!audioOutput || videoInputs.audioFrames().ptsInRange(state.nextPts))) {
    if (state.playbackState == PlaybackState::PLAYING) {
      togglePlaying();
      audioSeek(state.nextPts);
      togglePlaying();
    } else {
      audioSeek(state.nextPts);
      eventLoop.scheduleAdvanceFrame(5);
    }
  } else {
    state.seeking = true;
    videoInputs.seek(state.nextPts);
    eventLoop.scheduleAdvanceFrame(5);
  }
}

void VivictPP::audioSeek(double pts) {
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

void VivictPP::switchStream(int delta) {
  logger->debug("VivictPP::switchStream delta={}", delta);
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
    eventLoop.scheduleAdvanceFrame(5);
//    eventLoop.scheduleRefreshDisplay(0);
//  }
}

int VivictPP::run() {
  eventLoop.scheduleAdvanceFrame(5);
  eventLoop.start(*this);
  if (audioOutput) {
    audioOutput->stop();
  }
  logger->debug("VivictPP::run exit");
  return 0;
}

void VivictPP::mouseDragStart() {
  screenOutput.setCursorHand();
}

void VivictPP::mouseDragEnd() {
  screenOutput.setCursorDefault();
}

void VivictPP::mouseDrag(int xrel, int yrel) {
  state.displayState.panX -=
    (float)xrel / state.displayState.zoom.multiplier();
  state.displayState.panY -=
    (float)yrel / state.displayState.zoom.multiplier();
  eventLoop.scheduleRefreshDisplay(0);
}

void VivictPP::mouseMotion(int x, int y) {
  state.displayState.splitPercent =
    x * 100.0 / screenOutput.getWidth();
  eventLoop.scheduleRefreshDisplay(0);
}

void VivictPP::mouseWheel(int x, int y) {
  state.displayState.panX -=
    (float)10 * x / state.displayState.zoom.multiplier();
  state.displayState.panY -=
    (float)10 * y / state.displayState.zoom.multiplier();
  eventLoop.scheduleRefreshDisplay(0);
}

void VivictPP::mouseClick() {
  togglePlaying();
}

void VivictPP::onQuit() {
  eventLoop.stop();
}

void VivictPP::keyPressed(std::string key) {
  logger->debug("VivictPP::keyPressed key='{}'", key);
  if (key.length() == 1) {
    switch (key[0]) {
    case 'Q':
      onQuit();
      break;
    case '.':
      seekNextFrame();
      break;
    case ',':
      seekPreviousFrame();
      break;
    case '/':
      seekRelative(5);
      break;
    case 'M':
      seekRelative(-5);
      break;
    case 'U':
      state.displayState.zoom.increment();
      eventLoop.scheduleRefreshDisplay(0);
      logger->debug("Zoom: {}", state.displayState.zoom.get());
      break;
    case 'I':
      state.displayState.zoom.decrement();
      eventLoop.scheduleRefreshDisplay(0);
      logger->debug("Zoom: {}", state.displayState.zoom.get());
      break;
    case '0':
      state.displayState.zoom.set(0);
      state.displayState.panX = 0;
      state.displayState.panY = 0;
      eventLoop.scheduleRefreshDisplay(0);
      break;
    case 'F':
      state.displayState.fullscreen = !state.displayState.fullscreen;
      screenOutput.setFullscreen(state.displayState.fullscreen);
      break;
    case 'T':
      state.displayState.displayTime = !state.displayState.displayTime;
      eventLoop.scheduleRefreshDisplay(0);
      break;
    case 'D':
      state.displayState.displayMetadata = !state.displayState.displayMetadata;
      eventLoop.scheduleRefreshDisplay(0);
      break;
    case 'P':
      state.displayState.displayPlot = !state.displayState.displayPlot;
      eventLoop.scheduleRefreshDisplay(0);
      break;
    case '1':
      switchStream(-1);
      break;
    case '2':
      switchStream(1);
      break;
    }
  } else {
    if (key == "Space") {
      togglePlaying();
    }
  }
}
