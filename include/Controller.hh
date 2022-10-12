// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef CONTROLLER_HH_
#define CONTROLLER_HH_

#include "ui/DisplayState.hh"
#include "EventListener.hh"
#include "EventLoop.hh"
#include "ui/VivictPPUI.hh"
#include "logging/Logging.hh"
#include "VivictPP.hh"
#include <string>
#include "time/Time.hh"
#include "ui/Events.hh"

namespace vivictpp {

class Controller : vivictpp::EventListener {
public:
  Controller(std::shared_ptr<EventLoop> eventLoop,
             std::shared_ptr<vivictpp::ui::Display> display,
             VivictPPConfig vivictPPConfig);
  int run();
  void mouseDrag(const ui::MouseDragged mouseDragged) override;
  void mouseDragStarted(const ui::MouseDragStarted mouseDragStarted) override;
  void mouseDragStopped(const ui::MouseDragStopped mouseDragStopped) override;
  void mouseMotion(const int x, const int y) override;
  void mouseWheel(const int x, const int y) override;
  void mouseClick(const ui::MouseClicked mouseClicked) override;
  void keyPressed(const std::string &key, const vivictpp::KeyModifiers &modifiers) override;
  void advanceFrame() override;
  void refreshDisplay() override;
  void queueAudio() override;
  void fade() override;
  void seekFinished(vivictpp::time::Time seekedPos, bool error) override;
  void onQuit();
  const PlayerState &getPlayerState() { return vivictPP.getPlayerState(); }
  
private:
  void togglePlaying();
  void adjustPlaybackSpeed(int delta);

private:
  std::shared_ptr<EventLoop> eventLoop;
  std::shared_ptr<vivictpp::ui::Display> display;
  VivictPP vivictPP;
  vivictpp::ui::DisplayState displayState;
  bool splitScreenDisabled;
  bool plotEnabled;
  vivictpp::time::Time startTime;
  vivictpp::time::Time inputDuration;
  vivictpp::logging::Logger logger;
};

}  // vivictpp

#endif  // CONTROLLER_HH_
