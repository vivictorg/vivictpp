// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "Controller.hh"
#include "logging/Logging.hh"
#include "VideoMetadata.hh"
#include "sdl/SDLAudioOutput.hh"
#include "ui/Events.hh"

#include <algorithm>

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
    plotEnabled(false),
    startTime(vivictPP.getVideoInputs().startTime()),
    inputDuration(vivictPP.getVideoInputs().duration()),
    logger(vivictpp::logging::getOrCreateLogger("Controller")) {
  displayState.splitScreenDisabled = splitScreenDisabled;
  displayState.displayPlot = plotEnabled;
  displayState.leftVideoMetadata = vivictPP.getVideoInputs().metadata()[0][0];
  if (! (vivictPP.getVideoInputs().metadata()[1]).empty()) {
    displayState.rightVideoMetadata = vivictPP.getVideoInputs().metadata()[1][0];
  }
  displayState.videoMetadataVersion++;
  eventLoop->scheduleRefreshDisplay(0);
}

int vivictpp::Controller::run() {
  eventLoop->start(*this);
  logger->debug("vivictpp::Controller::run exit");
  return 0;
}

void vivictpp::Controller::advanceFrame() {
  vivictPP.advanceFrame();
}


void vivictpp::Controller::fade() {
  if (displayState.seekBar.hideTimer > 0 && displayState.seekBar.visible) {
    displayState.seekBar.opacity = std::max(0, displayState.seekBar.opacity - 10);
    if (displayState.seekBar.opacity == 0) {
      displayState.seekBar.visible = false;
      displayState.seekBar.hideTimer = 0;
      displayState.seekBar.opacity = 255;
    } else {
      eventLoop->scheduleFade(40);
    }
    if (! vivictPP.isPlaying()) {
      eventLoop->scheduleRefreshDisplay(0);
    }
  }
}

void vivictpp::Controller::seekFinished(vivictpp::time::Time seekedPos, bool error) {
  vivictPP.onSeekFinished(seekedPos, error);
  displayState.seekBar.seeking = false;
}

void vivictpp::Controller::refreshDisplay() {
  logger->trace("vivictpp::Controller::refreshDisplay");
  std::array<vivictpp::libav::Frame, 2> frames = vivictPP.getVideoInputs().firstFrames();
  displayState.leftFrame = frames[0];
  displayState.rightFrame = frames[1];
  if (displayState.displayTime) {
    displayState.timeStr = vivictpp::time::formatTime(vivictPP.getPts());
  }
  displayState.pts = vivictPP.getPts();
  displayState.seekBar.relativePos = (displayState.pts - startTime) / (float) inputDuration;
  display->displayFrame(displayState);
}


void vivictpp::Controller::queueAudio() {
  vivictPP.queueAudio();
}

void vivictpp::Controller::mouseDrag(const ui::MouseDragged mouseDragged) {
  logger->debug("vivictpp::Controller::mouseDrag target={}", mouseDragged.target);
    if (mouseDragged.target == "seekbar") {
      auto box = mouseDragged.component.get().getBox();
      double seekRel = (mouseDragged.x - box.x) / (double) box.w;
      logger->trace("vivictpp::Controller::mouseDrag seekRel={}", seekRel);
      logger->trace("vivictpp::Controller::mouseDrag mouseDragged.x={}, box.x={}, box.w={}",
                    mouseDragged.x, box.x, box.w);
      displayState.seekBar.relativeSeekPos = std::min(1.0, std::max(seekRel, 0.0));
      eventLoop->scheduleRefreshDisplay(0);
    } else {
      int xrel = mouseDragged.xrel;
      int yrel = mouseDragged.yrel;
      displayState.panX -=
        (float)xrel / displayState.zoom.multiplier();
      displayState.panY -=
        (float)yrel / displayState.zoom.multiplier();
      eventLoop->scheduleRefreshDisplay(0);
    }
}

void vivictpp::Controller::mouseDragStopped(const ui::MouseDragStopped mouseDragStopped) {
  if (mouseDragStopped.target == "seekbar") {
    float pos = startTime + inputDuration * displayState.seekBar.relativeSeekPos;
    logger->debug("seeking to {}", pos);
    vivictPP.seek(pos);
    displayState.seekBar.seeking = false;
    eventLoop->scheduleRefreshDisplay(0);
  }
}

void vivictpp::Controller::mouseDragStarted(const ui::MouseDragStarted mouseDragStarted) {
  if (mouseDragStarted.target == "seekbar") {
    auto box = mouseDragStarted.component.get().getBox();
    double seekRel = (mouseDragStarted.x - box.x) / (double) box.w;
    displayState.seekBar.relativeSeekPos = std::min(1.0, std::max(seekRel, 0.0));
    displayState.seekBar.seeking = true;
    eventLoop->scheduleRefreshDisplay(0);
  }
}

void vivictpp::Controller::mouseMotion(const int x, const int y) {
  logger->trace("vivictpp::Controller::mouseMotion x={} y={}", x, y);
  (void) y;
  displayState.splitPercent =
    x * 100.0 / display->getWidth();
  bool showSeekBar = y > display->getHeight() - 70;
  logger->trace("vivictpp::Controller::mouseMotion y={}, display->getHeight()={} showSeekBar={}",
                y, display->getHeight(), showSeekBar);
  if (displayState.seekBar.visible && !showSeekBar && displayState.seekBar.hideTimer == 0) {
     displayState.seekBar.hideTimer = vivictpp::time::relativeTimeMillis() + 500;
     fade();
  } else if (showSeekBar) {
    displayState.seekBar.visible = true;
    displayState.seekBar.hideTimer = 0;
    displayState.seekBar.opacity = 255;
  }
  eventLoop->scheduleRefreshDisplay(0);
}

void vivictpp::Controller::mouseWheel(const int x, const int y) {
  displayState.panX -=
    (float)10 * x / displayState.zoom.multiplier();
  displayState.panY -=
    (float)10 * y / displayState.zoom.multiplier();
  eventLoop->scheduleRefreshDisplay(0);
}

void vivictpp::Controller::mouseClick(const vivictpp::ui::MouseClicked mouseClicked) {
  logger->debug("vivictpp::Controller::mouseClick {}", mouseClicked.target);
  if (mouseClicked.target == "seekbar") {
    auto box = mouseClicked.component.get().getBox();
    float seekRel = (mouseClicked.x - box.x) / (float) box.w;
    float pos = startTime + inputDuration * seekRel;
    logger->debug("seeking to {}", pos);
    vivictPP.seek(pos);
  } else {
    togglePlaying();
  }
}

void vivictpp::Controller::togglePlaying() {
  displayState.isPlaying = vivictPP.togglePlaying() == PlaybackState::PLAYING;
}

void vivictpp::Controller::onQuit() {
  eventLoop->stop();
  vivictPP.onQuit();
}

int seekDistance(const vivictpp::KeyModifiers &modifiers) {
  return modifiers.shift ? (modifiers.alt ? 600 : 60) : 5;
}

void vivictpp::Controller::keyPressed(const std::string &key, const vivictpp::KeyModifiers &modifiers) {
  logger->debug("vivictpp::Controller::keyPressed key='{}' shift={} ctrl={}", key, modifiers.shift, modifiers.ctrl);
  if (key.length() == 1) {
    switch (key[0]) {
    case 'Q':
      onQuit();
      break;
    case '.':
      if (modifiers.shift) {
        displayState.leftFrameOffset = vivictPP.increaseFrameOffset();
      } else {
        vivictPP.seekNextFrame();
      }
      break;
    case ',':
      if (modifiers.shift) {
        displayState.leftFrameOffset = vivictPP.decreaseFrameOffset();
      } else {
        vivictPP.seekPreviousFrame();
      }
      break;
    case '/':
      vivictPP.seekRelative(vivictpp::time::seconds(seekDistance(modifiers)));
      break;
    case 'M':
      vivictPP.seekRelative(vivictpp::time::seconds(-1 * seekDistance(modifiers)));
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
    case 'S':
      displayState.fitToScreen = !displayState.fitToScreen;
      eventLoop->scheduleRefreshDisplay(0);
      break;
    case '1':
      vivictPP.switchStream(-1);
      break;
    case '2':
      vivictPP.switchStream(1);
      break;
    case '[':
      adjustPlaybackSpeed(1);
      break;
    case ']':
      adjustPlaybackSpeed(-1);
      break;
    }
  } else {
    if (key == "Space") {
      togglePlaying();
    }
  }
}

void vivictpp::Controller::adjustPlaybackSpeed(int delta) {
  int speed = vivictPP.adjustPlaybackSpeed(delta);
  if (speed == 0) {
    displayState.playbackSpeedStr = "";
  } else {
    float speedFloat = std::pow(std::sqrt(2), -1 * speed);
    displayState.playbackSpeedStr =  fmt::format("{:.2f}", speedFloat);
  }
  eventLoop->scheduleRefreshDisplay(0);
}
