// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIVICTPP_UI_EVENTS_H_
#define VIVICTPP_UI_EVENTS_H_

#include <memory>
#include <string>

#include "ui/Ui.hh"


namespace vivictpp {
namespace ui {

class MouseClicked {
public:
    std::string target;
    int x;
    int y;
    std::reference_wrapper<Component> component;
};

class MouseDragged {
public:
    std::string target;
    int x;
    int y;
    int xrel;
    int yrel;
    std::reference_wrapper<Component> component;
};

class MouseDragStarted {
public:
  std::string target;
  int x;
  int y;
  std::reference_wrapper<Component> component;
};

class MouseDragStopped {
public:
    std::string target;
    std::reference_wrapper<Component> component;
};

}  // namespace ui
}  // namespace vivictpp


#endif  // VIVICTPP_UI_EVENTS_H_
