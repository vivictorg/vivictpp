// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ui/TextTexture.hh"
#include "ui/Fonts.hh"
#include <stdexcept>

SDL_Surface *createSurface(std::string text,
                          int fontSize,
                           SDL_Color color,
                           std::string font) {
  SDL_Surface *surface =
    TTF_RenderText_Blended(vivictpp::ui::Fonts::getFont(font, fontSize), text.c_str(), color);
  if (surface == nullptr) {
    throw std::runtime_error(std::string("TTF_RenderText_Blended: ") +
                             TTF_GetError());
  }
  return surface;
}

vivictpp::ui::TextTexture::TextTexture(SDL_Renderer *renderer,
                                       std::string text,
                                       int fontSize,
                                       SDL_Color color,
                                       std::string font):
  surface(createSurface(text, fontSize, color, font)),
  texture(SDL_CreateTextureFromSurface(renderer, surface), &SDL_DestroyTexture),
  width(surface->w),
  height(surface->h)
{
  SDL_FreeSurface(surface);
  surface = nullptr;
}

void vivictpp::ui::TextTexture::render(SDL_Renderer *renderer, int x, int y) const {
  SDL_Rect rect{x,y, width, height};
  SDL_RenderCopy(renderer, texture.get(), nullptr, &rect);
}
