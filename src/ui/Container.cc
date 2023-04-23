// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ui/Container.hh"
#include <cstddef>

void vivictpp::ui::Container::render(const DisplayState &displayState, SDL_Renderer *renderer, int x, int y) {
  box.x = x;
  box.y = y;
  int w = 0;
  for (size_t i = 0; i < components.size(); i++) {
    auto &c = components[i];
    c->render(displayState, renderer, x, y);
    if (c->getBox().h > 0) {
      y += c->getBox().h + padding;
    }
    w = std::max(w, c->getBox().w);
  }
  box.h = std::max(0, y - padding);
  box.w = w;
}

vivictpp::ui::Component& vivictpp::ui::Container::operator[](int index) {
  return *components[index];
}

vivictpp::ui::FixedPositionContainer::FixedPositionContainer(
  vivictpp::ui::Position position,
  std::vector<std::shared_ptr<vivictpp::ui::Component>> components,
  vivictpp::ui::Offset offset):
  Container(components),
  position(position),
  offset(offset) { }

void vivictpp::ui::FixedPositionContainer::render(const DisplayState &displayState, SDL_Renderer *renderer, int x, int y) {
  int rendererWidth;
  int rendererHeight;
  SDL_GetRendererOutputSize(renderer, &rendererWidth, &rendererHeight);

  x = 0 + this->offset.x;
  y = 5 + this->offset.y;
  switch (position) {
  case Position::Absolute:
    x = this->offset.x;
    y = this->offset.y;
    break;
  case Position::TopLeft:
    x = 5 + this->offset.x;
    break;
  case Position::TopCenter:
    x = this->offset.x + (rendererWidth - box.w) / 2;
    break;
  case Position::TopRight:
    x = this->offset.x + rendererWidth - box.w - 5;
    break;
  case Position::Center:
    x = this->offset.x + (rendererWidth - box.w) / 2;
    y = this->offset.y + (rendererHeight - box.h) / 2;
  }

  Container::render(displayState, renderer, x, y);
}

