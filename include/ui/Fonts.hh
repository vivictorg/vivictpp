// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef UI_FONTS_HH
#define UI_FONTS_HH


extern "C" {
#include <SDL.h>
#include <SDL_ttf.h>
}

#include <map>
#include <string>

namespace vivictpp {
namespace ui {

class Fonts  {
public:
  static TTF_Font *getFont(std::string font, int size);

private:
  static Fonts& instance() {
    static Fonts _instance;
    return _instance;
  }
  Fonts();
  ~Fonts();
  Fonts(Fonts const&) = delete;
  void operator=(Fonts const&) = delete;
  TTF_Font *getFontInternal(std::string font, int size);
  std::map<std::pair<std::string, int>, TTF_Font *> fonts;
};

}  // namespace ui
}  // namespace vivictpp

#endif // UI_FONTS_HH
