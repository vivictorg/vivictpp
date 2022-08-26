// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef UI_VIVICTPPUI_HH
#define UI_VIVICTPPUI_HH

#include "libav/Frame.hh"
#include "VideoMetadata.hh"
#include "EventLoop.hh"
#include "ui/DisplayState.hh"
#include <string>

namespace vivictpp {
namespace ui {

/*
struct MouseClick {
  const std::string target;
  const int x;
  const int y;
};

struct MouseDrag {
  const std::string target;
  const int xrel;
  const int yrel;
};

struct MouseMotion {
  std::string target;
  int x;
  int y;
};

struct MouseClick {
  std::string target;
  int x;
  int y;
};
  
*/

class Display {
public:
  virtual ~Display() = default;
  virtual void displayFrame(const vivictpp::ui::DisplayState &displayState) = 0;
  virtual int getWidth() = 0;
  virtual int getHeight() = 0;
  virtual void setFullscreen(bool fullscreen) = 0;
//  void setCursorHand();
//  void setCursorDefault();
};


class VivictPPUI: public Display, public EventLoop {

};


}  // ui
}  // vivictpp

#endif // UI_VIVICTPPUI_HH
