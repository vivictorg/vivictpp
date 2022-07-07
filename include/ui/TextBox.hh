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

#include "ui/Ui.hh"

namespace vivictpp {
namespace ui {

class TextBox: public Component {
public:
  TextBox(std::string text, std::string font, int fontSize,
          std::string title="",
          int minWidth = 0, int minHeight = 0,
          Margin margin = {2,2,2,2});
  ~TextBox();
  void render(SDL_Renderer *renderer, int x, int y) override;
  int getRenderedWidth() override { return display ? textureW : 0; }
  int getRenderedHeight() override { return display ? textureH : 0; }
  void setText(std::string newText);
  SDL_Color fg = {255, 255, 255, 255};
  SDL_Color bg = {50, 50, 50, 255};
  bool border = true;
  bool display = true;

private:
  SDL_Texture *texture;
  std::string text;
  int textureW = 0;
  int textureH = 0;
  const std::string font;
  const int fontSize;
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
