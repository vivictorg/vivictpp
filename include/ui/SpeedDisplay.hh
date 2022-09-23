// SPDX-FileCopyrightText: 2022 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef UI_SPEEDDISPLAY_HH
#define UI_SPEEDDISPLAY_HH

#include "ui/DisplayState.hh"
#include "ui/FrameMetadataBox.hh"
#include "ui/TextBox.hh"
#include "ui/Ui.hh"
#include "ui/Container.hh"
#include <memory>

namespace vivictpp::ui {

class TimeDisplay : public TextBox {
public:
  TimeDisplay() :
    TextBox("00:00:00", "FreeMono", 24) {}

  void render(const DisplayState &displayState, SDL_Renderer *renderer, int x, int y) override {
    display = displayState.displayTime;
    if (display) {
      setText(displayState.timeStr);
    }
    TextBox::render(displayState, renderer, x, y);
  }

};

class SpeedDisplay : public TextBox {
public:
  SpeedDisplay():
    TextBox("", "FreeMono", 18),
    speedStr("") {
    bg = {50,50,50,127};
    border = false;
  };

  void render(const DisplayState &displayState, SDL_Renderer *renderer, int x, int y) override {
    display = !displayState.playbackSpeedStr.empty();
    if (display && displayState.playbackSpeedStr != speedStr) {
      speedStr = displayState.playbackSpeedStr;
      setText(std::string("Speed: x") + speedStr);
    }
    TextBox::render(displayState, renderer, x, y);
  };

private:
  std::string speedStr;
};

}

#endif // UI_SPEEDDISPLAY_HH
