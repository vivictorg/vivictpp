// SPDX-FileCopyrightText: 2020 Sveriges Television AB
// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef UI_CONTAINER_HH
#define UI_CONTAINER_HH

#include <vector>
#include <memory>
#include <string>

extern "C" {
#include <SDL.h>
#include <SDL_ttf.h>
}

#include "ui/Ui.hh"

namespace vivictpp {
namespace ui {

class Container: public Component {
public:
    Container(std::vector<std::shared_ptr<vivictpp::ui::Component>> components = {}):
        components(components) {};
    virtual ~Container() = default;
    void render(SDL_Renderer *renderer, int x, int y) override;
    const Box& getBox() const override { return box; }
    void add(std::shared_ptr<Component> component) {
        components.push_back(component);
    }
    Component& operator[](int index);
    template <typename T> T& getComponent(int index) {
        return dynamic_cast<T&>(*components[index]);
    }
protected:
    std::vector<std::shared_ptr<Component>> components;
    int padding = 2;
    Box box;
};

class FixedPositionContainer: public Container {
public:
    FixedPositionContainer(Position position = Position::TOP_LEFT,
                           std::vector<std::shared_ptr<Component>> components = {},
                           Offset offset = {0,0});
    virtual ~FixedPositionContainer() = default;
    virtual void render(SDL_Renderer *renderer, int x = 0, int y = 0);
private:
    Position position;
    Offset offset;
};

}  // namespace ui
}  // namespace vivictpp


#endif // UI_CONTAINER_HH
