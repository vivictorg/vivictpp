// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef UI_TEXTTEXTURE_HH
#define UI_TEXTTEXTURE_HH

extern "C" {
#include <SDL.h>
#include <SDL_ttf.h>
}
#include <string>
#include <memory>

namespace vivictpp {
namespace ui {

class TextTexture {
private:
  SDL_Surface* surface;

public:
  TextTexture(SDL_Renderer *renderer,
              std::string text,
              int fontSize,
              SDL_Color color = {0,0,0,0},
              std::string font = "FreeMono");
  ~TextTexture() = default;
  std::shared_ptr<SDL_Texture> texture;
  const int width;
  const int height;
  void render(SDL_Renderer *renderer, int x, int y) const;
};

}  // namespace vivictpp
}  // namespace ui


#endif // UI_TEXTTEXTURE_HH
