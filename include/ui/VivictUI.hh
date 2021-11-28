// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIVICT_UI_HH_
#define VIVICT_UI_HH_

#include "ui/DisplayState.hh"
#include "EventListener.hh"
#include "logging/Logging.hh"
#include "VivictPP.hh"

namespace vivictpp {
namespace ui {

class VivictUI : EventListener {
public:
  VivictUI(VivictPPConfig vivictPPConfig);
  int run();
  void mouseDragStart() override;
  void mouseDragEnd() override;
  void mouseDrag(int xrel, int yrel) override;
  void mouseMotion(int x, int y) override;
  void mouseWheel(int x, int y) override;
  void mouseClick(int x, int y) override;
  void keyPressed(std::string key) override;
  void advanceFrame() override;
  void refreshDisplay() override;
  void queueAudio() override;
  void fade() override;
  void onQuit();

private:
  void togglePlaying();

private:
  vivictpp::sdl::SDLEventLoop eventLoop;
  VivictPP vivictPP;
  vivictpp::ui::DisplayState displayState;
  vivictpp::ui::ScreenOutput screenOutput;
  bool splitScreenDisabled;
  bool plotEnabled;
  double startTime;
  double inputDuration;
  vivictpp::logging::Logger logger;
};

}  // ui
}  // vivictpp

#endif  // VIVICT_UI_HH_
