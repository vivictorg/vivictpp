// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "Controller.hh"
#include "logging/Logging.hh"
#include "VideoMetadata.hh"
#include "sdl/SDLAudioOutput.hh"

VideoMetadata* metadataPtr(const std::vector<VideoMetadata> &v) {
  if (v.empty()) {
    return nullptr;
  }
  return new VideoMetadata(v[0]);
}


vivictpp::Controller::Controller(std::shared_ptr<EventLoop> eventLoop,
                                 std::shared_ptr<vivictpp::ui::Display> display,
                                 VivictPPConfig vivictPPConfig)
  : eventLoop(eventLoop),
    display(display),
    vivictPP(vivictPPConfig, eventLoop, vivictpp::sdl::audioOutputFactory),
    splitScreenDisabled(vivictPPConfig.sourceConfigs.size() == 1),
    plotEnabled(vivictPPConfig.hasVmafData()),
    startTime(vivictPP.getVideoInputs().startTime()),
    inputDuration(vivictPP.getVideoInputs().duration()),
    logger(vivictpp::logging::getOrCreateLogger("Controller")) {
  displayState.splitScreenDisabled = splitScreenDisabled;
  displayState.displayPlot = plotEnabled;
  display->setLeftMetadata(vivictPP.getVideoInputs().metadata()[0][0]);
  if (! (vivictPP.getVideoInputs().metadata()[1]).empty()) {
    display->setRightMetadata(vivictPP.getVideoInputs().metadata()[1][0]);
  }
}

int vivictpp::Controller::run() {
//  eventLoop.scheduleAdvanceFrame(5);
  eventLoop->start(*this);
  logger->debug("vivictpp::Controller::run exit");
  return 0;
}

void vivictpp::Controller::advanceFrame() {
  vivictPP.advanceFrame();
}


void vivictpp::Controller::fade() {
  if (displayState.hideSeekBar > 0 && displayState.seekBarVisible) {
    displayState.seekBarOpacity = std::max(0, displayState.seekBarOpacity - 10);
    if (displayState.seekBarOpacity == 0) {
      displayState.seekBarVisible = false;
      displayState.hideSeekBar = 0;
      displayState.seekBarOpacity = 255;
    } else {
      eventLoop->scheduleFade(40);
    }
    if (! vivictPP.isPlaying()) {
      eventLoop->scheduleRefreshDisplay(0);
    }
  }
}

void vivictpp::Controller::refreshDisplay() {
  logger->trace("vivictpp::Controller::refreshDisplay");
  std::array<vivictpp::libav::Frame, 2> frames = vivictPP.getVideoInputs().firstFrames();
  if (displayState.displayTime) {
    displayState.timeStr = vivictpp::util::formatTime(vivictPP.getPts());
  }
  displayState.pts = vivictPP.getPts();
  // TODO: Take start time into consideration
  displayState.seekBarRelativePos = (displayState.pts - startTime) / inputDuration;
  display->displayFrame(frames, displayState);
}


void vivictpp::Controller::queueAudio() {
  vivictPP.queueAudio();
}

void vivictpp::Controller::mouseDrag(int xrel, int yrel) {
  displayState.panX -=
    (float)xrel / displayState.zoom.multiplier();
  displayState.panY -=
    (float)yrel / displayState.zoom.multiplier();
  eventLoop->scheduleRefreshDisplay(0);
}

void vivictpp::Controller::mouseMotion(int x, int y) {
  (void) y;
  displayState.splitPercent =
    x * 100.0 / display->getWidth();
  bool showSeekBar = y > display->getHeight() - 70;
  if (displayState.seekBarVisible && !showSeekBar && displayState.hideSeekBar == 0) {
     displayState.hideSeekBar = vivictpp::util::relativeTimeMillis() + 500;
     fade();
  } else if (showSeekBar) {
    displayState.seekBarVisible = true;
    displayState.hideSeekBar = 0;
    displayState.seekBarOpacity = 255;
  }
  eventLoop->scheduleRefreshDisplay(0);
}

void vivictpp::Controller::mouseWheel(int x, int y) {
  displayState.panX -=
    (float)10 * x / displayState.zoom.multiplier();
  displayState.panY -=
    (float)10 * y / displayState.zoom.multiplier();
  eventLoop->scheduleRefreshDisplay(0);
}

void vivictpp::Controller::mouseClick(int x, int y /*, std::string target*/) {
  (void) x;
  (void) y;
  /*
  vivictpp::ui::ClickTarget clickTarget = eventLoop.getClickTarget(x, y, displayState);
  if (clickTarget.name == "plot" || clickTarget == "seekbar") {
    float seekRel = (x - clickTarget.x) / (float) clickTarget.w;
    float pos = startTime + inputDuration * seekRel;
    logger->debug("seeking to {}", pos);
    vivictPP.seek(pos);
  } else {
  */
  togglePlaying();
//  }
}

void vivictpp::Controller::togglePlaying() {
  displayState.isPlaying = vivictPP.togglePlaying() == PlaybackState::PLAYING;
}

void vivictpp::Controller::onQuit() {
  eventLoop->stop();
  vivictPP.onQuit();
}

void vivictpp::Controller::keyPressed(std::string key) {
  logger->debug("vivictpp::Controller::keyPressed key='{}'", key);
  if (key.length() == 1) {
    switch (key[0]) {
    case 'Q':
      onQuit();
      break;
    case '.':
      vivictPP.seekNextFrame();
      break;
    case ',':
      vivictPP.seekPreviousFrame();
      break;
    case '/':
      vivictPP.seekRelative(5);
      break;
    case 'M':
      vivictPP.seekRelative(-5);
      break;
    case 'U':
      displayState.zoom.increment();
      eventLoop->scheduleRefreshDisplay(0);
      logger->debug("Zoom: {}", displayState.zoom.get());
      break;
    case 'I':
      displayState.zoom.decrement();
      eventLoop->scheduleRefreshDisplay(0);
      logger->debug("Zoom: {}", displayState.zoom.get());
      break;
    case '0':
      displayState.zoom.set(0);
      displayState.panX = 0;
      displayState.panY = 0;
      eventLoop->scheduleRefreshDisplay(0);
      break;
    case 'F':
      displayState.fullscreen = !displayState.fullscreen;
      display->setFullscreen(displayState.fullscreen);
      break;
    case 'T':
      displayState.displayTime = !displayState.displayTime;
      eventLoop->scheduleRefreshDisplay(0);
      break;
    case 'D':
      displayState.displayMetadata = !displayState.displayMetadata;
      eventLoop->scheduleRefreshDisplay(0);
      break;
    case 'P':
      displayState.displayPlot = !displayState.displayPlot;
      eventLoop->scheduleRefreshDisplay(0);
      break;
    case '1':
      vivictPP.switchStream(-1);
      break;
    case '2':
      vivictPP.switchStream(1);
      break;
    }
  } else {
    if (key == "Space") {
      togglePlaying();
    }
  }
}
