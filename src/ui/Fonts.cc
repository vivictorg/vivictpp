// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ui/Fonts.hh"
#include "ui/FreeMono.hh"

vivictpp::ui::Fonts::Fonts() {
  TTF_Init();
}

vivictpp::ui::Fonts::~Fonts() {
  for (std::pair<std::pair<std::string, int>, TTF_Font *> element : fonts) {
    TTF_CloseFont(element.second);
  }
  TTF_Quit();
}

TTF_Font* vivictpp::ui::Fonts::getFont(std::string font, int size) {
  return instance().getFontInternal(font, size);
}

TTF_Font* vivictpp::ui::Fonts::getFontInternal(std::string font, int size) {
  std::pair<std::string, int> key(font, size);
  if (fonts[key] == nullptr) {
    if (font == "FreeMono") {
      fonts[key] = TTF_OpenFontRW(
          SDL_RWFromConstMem(FreeMono_ttf, FreeMono_ttf_len), 1, size);
    } else {
      fonts[key] = TTF_OpenFont(font.c_str(), size);
    }
    if (!fonts[key])
      throw std::runtime_error(std::string("TTF_OpenFont: ") + TTF_GetError());
  }
  return fonts[key];
}
