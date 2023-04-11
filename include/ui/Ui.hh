// SPDX-FileCopyrightText: 2020 Sveriges Television AB
// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef UI_UI_HH
#define UI_UI_HH

//extern "C" {
//#include <SDL.h>
//#include <SDL_ttf.h>
//}

#include "ui/DisplayState.hh"

struct SDL_Renderer;

namespace vivictpp::ui {

enum class Position { TopLeft, TopCenter, TopRight, Center, Absolute };

enum class DisplayMode { Visible, Hidden, None };

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

}  // namespace vivictpp::ui



#endif // UI_UI_HH
