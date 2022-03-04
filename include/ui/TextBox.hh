// SPDX-FileCopyrightText: 2020 Sveriges Television AB
// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef TEXTBOX_HH_
#define TEXTBOX_HH_

#include <libavutil/pixfmt.h>
extern "C" {
#include <SDL.h>
#include <SDL_ttf.h>
}

#include <map>
#include <math.h>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace vivictpp {
namespace ui {

enum class TextBoxPosition { TOP_LEFT, TOP_CENTER, TOP_RIGHT, CENTER, ABSOLUTE };

struct Margin {
  int top;
  int right;
  int bottom;
  int left;
};

class TextBox {
public:
  TextBox(std::string text, std::string font, int fontSize,
          TextBoxPosition position, int x=0, int y=0, std::string title="",
          int minWidth = 0, int minHeight = 0,
          Margin margin = {2,2,2,2});
  ~TextBox();
  void render(SDL_Renderer *renderer);
  void setText(std::string newText);
  SDL_Color fg = {255, 255, 255, 255};
  SDL_Color bg = {50, 50, 50, 255};
  bool border = true;
  void setY(int _y) { y = _y; }
private:
  SDL_Texture *texture;
  std::string text;
  int textureW = 0;
  int textureH = 0;
  const std::string font;
  const int fontSize;
  const TextBoxPosition position;
  int x = 0;
  int y = 0;
  std::string title;
  int minWidth = 0;
  int minHeight = 0;
  Margin margin;
  bool changed = false;
private:
  void initTexture(SDL_Renderer *renderer);
};

}  // ui
}  // vivictpp

#endif // TEXTBOX_HH_
