// SPDX-FileCopyrightText: 2020 Sveriges Television AB
// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef UI_UI_HH
#define UI_UI_HH

extern "C" {
#include <SDL.h>
#include <SDL_ttf.h>
}

#include "ui/DisplayState.hh"

namespace vivictpp {
namespace ui {


enum class Position { TOP_LEFT, TOP_CENTER, TOP_RIGHT, CENTER, ABSOLUTE };

enum class DisplayMode { VISIBLE, HIDDEN, NONE };

struct Offset {
  int x;
  int y;
};

struct Margin {
  int top;
  int right;
  int bottom;
  int left;
};

struct Box {
  int x;
  int y;
  int w;
  int h;
  bool contains(int x, int y) const {
    return x >= this->x && x < this->x + this->w && y >= this->y && y < this->y + this->h;
  }
};

class Component {
public:
  virtual ~Component() = default;
  virtual void render(const DisplayState &displayState, SDL_Renderer *renderer, int x, int y) = 0;
  virtual const Box& getBox() const = 0;
};

}  // namespace ui
}  // namespace vivictpp


#endif // UI_UI_HH
