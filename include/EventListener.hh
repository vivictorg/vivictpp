// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef EVENT_LISTENER_H_
#define EVENT_LISTENER_H_

#include <cstdint>
#include <string>

#include "ui/Events.hh"
#include "time/Time.hh"

namespace vivictpp {

struct KeyModifiers {
  bool shift: 1;
  bool ctrl: 1;
  bool alt: 1;
};

class EventListener {
 public:
  virtual ~EventListener() = default;
  virtual void mouseDrag(const vivictpp::ui::MouseDragged mouseDragged) = 0;
  virtual void mouseDragStarted(const vivictpp::ui::MouseDragStarted mouseDragStarted) = 0;
  virtual void mouseDragStopped(const vivictpp::ui::MouseDragStopped mouseDragStopped) = 0;
  virtual void mouseMotion(const int x, const int y) = 0;
  virtual void mouseWheel(const int x, const int y) = 0;
  virtual void mouseClick(const vivictpp::ui::MouseClicked mouseClicked) = 0;
  virtual void keyPressed(const std::string &key, const KeyModifiers &modifiers) = 0;
  virtual void advanceFrame() = 0;
  virtual void queueAudio() = 0;
  virtual void refreshDisplay() = 0;
  virtual void fade() = 0;
  virtual void seekFinished(vivictpp::time::Time seekedPos) = 0;

};

}  // namespace vivictpp

#endif  // #ifndef EVENT_LISTENER_H_
