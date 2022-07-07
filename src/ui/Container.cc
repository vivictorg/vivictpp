#include "ui/Container.hh"

int vivictpp::ui::Container::getRenderedWidth() {
  int w = 0;
  for (auto& c : components) {
    w = std::max(c->getRenderedWidth(), w);
  }
  return w;
}

int vivictpp::ui::Container::getRenderedHeight() {
  int h = 0;
  int n = 0;
  for (auto& c : components) {
    int ch = c->getRenderedHeight();
    h += ch;
    if (ch > 0) {
      n++;
    }
  }
  if (n > 1) {
      h += (n - 1) * padding;
  }
  return h;
}

void vivictpp::ui::Container::render(SDL_Renderer *renderer, int x, int y) {
  for (auto &c : components) {
    c->render(renderer, x, y);
    int h = c->getRenderedHeight();
    if (h > 0) {
      y += h  + padding;
    }
  }
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

void vivictpp::ui::FixedPositionContainer::render(SDL_Renderer *renderer, int x, int y) {
  int rendererWidth;
  int rendererHeight;
  SDL_GetRendererOutputSize(renderer, &rendererWidth, &rendererHeight);

  x = 0 + this->offset.x;
  y = 5 + this->offset.y;
  switch (position) {
  case Position::ABSOLUTE:
    x = this->offset.x;
    y = this->offset.y;
    break;
  case Position::TOP_LEFT:
    x = 5 + this->offset.x;
    break;
  case Position::TOP_CENTER:
    x = this->offset.x + (rendererWidth - getRenderedWidth()) / 2;
    break;
  case Position::TOP_RIGHT:
    x = this->offset.x + rendererWidth - getRenderedWidth() - 5;
    break;
  case Position::CENTER:
    x = this->offset.x + (rendererWidth - getRenderedWidth()) / 2;
    y = this->offset.y + (rendererHeight - getRenderedHeight()) / 2;
  }

  Container::render(renderer, x, y);
}

