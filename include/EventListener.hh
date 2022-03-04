// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef EVENT_LISTENER_H_
#define EVENT_LISTENER_H_

#include <string>

namespace vivictpp {

struct KeyModifiers {
  bool shift: 1;
  bool ctrl: 1;
};

class EventListener {
 public:
  virtual ~EventListener() = default;
  virtual void mouseDrag(const int xrel, const int yrel) = 0;
  virtual void mouseMotion(const int x, const int y) = 0;
  virtual void mouseWheel(const int x, const int y) = 0;
  virtual void mouseClick(const int x, const int y) = 0;
  virtual void keyPressed(const std::string &key, const KeyModifiers &modifiers) = 0;
  virtual void advanceFrame() = 0;
  virtual void queueAudio() = 0;
  virtual void refreshDisplay() = 0;
  virtual void fade() = 0;

};

}  // namespace vivictpp

#endif  // #ifndef EVENT_LISTENER_H_
