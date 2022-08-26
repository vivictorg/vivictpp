// SPDX-FileCopyrightText: 2022 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ui/Container.hh"
#include "ui/Ui.hh"
#include "ui/TextBox.hh"

#ifndef UI_SPLASH_HH
#define UI_SPLASH_HH

const std::string SPLASH_TEXT("____   ____._______   ____._______________________                       \n\\   \\ /   /|   \\   \\ /   /|   \\_   ___ \\__    ___/    .__         .__    \n \\   Y   / |   |\\   Y   / |   /    \\  \\/ |    |     __|  |___   __|  |___\n  \\     /  |   | \\     /  |   \\     \\____|    |    /__    __/  /__    __/\n   \\___/   |___|  \\___/   |___|\\______  /|____|       |__|        |__|   ");


namespace vivictpp::ui {

class Splash : public Component {
public:
  Splash():
    textBox(std::make_shared<TextBox>(SPLASH_TEXT, "FreeMono", 32)),
    container(Position::CENTER, {textBox}) {
     textBox->bg = {0,0,0,255};
     textBox->border = false;
  };
  void render(const DisplayState &displayState, SDL_Renderer *renderer, int x, int y) {
    container.render(displayState, renderer, x, y);
  };
  const Box& getBox() const {
    return container.getBox();
  }

private:
  std::shared_ptr<TextBox> textBox;
  FixedPositionContainer container;
};


}; // namespace vivictpp::ui


#endif // UI_SPLASH_HH
