// SPDX-FileCopyrightText: 2022 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef SEEKBAR_HH
#define SEEKBAR_HH

#include "ui/DisplayState.hh"
#include "ui/Ui.hh"

namespace vivictpp {
namespace ui {

class SeekBar: public Component {
public:
    SeekBar(Margin margin={0,0,0,0}):
        margin(margin){ }

    void render(SDL_Renderer *renderer, int x, int y) override;
    const Box& getBox() const override { return box; }
    int preferredHeight() { return seekBarHeight + margin.top + margin.bottom; }
  void setState(const SeekBarState &state) {
    this->state = state;
  }
private:
  SeekBarState state;
  Margin margin;
  Box box{0,0,0,0};
  int seekBarHeight{8};
};

}  // namespace vivictpp
}  // namespace ui

#endif
