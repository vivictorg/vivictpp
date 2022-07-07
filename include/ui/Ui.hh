// SPDX-FileCopyrightText: 2020 Sveriges Television AB
// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef UI_HH_
#define UI_HH_

extern "C" {
#include <SDL.h>
#include <SDL_ttf.h>
}

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

class Component {
public:
    virtual ~Component() = default;
    virtual void render(SDL_Renderer *renderer, int x, int y) = 0;
    virtual int getRenderedWidth() = 0;
    virtual int getRenderedHeight() = 0;
};

}  // namespace ui
}  // namespace vivictpp


#endif  // UI_HH_
