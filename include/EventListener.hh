// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef EVENT_LISTENER_H_
#define EVENT_LISTENER_H_

#include <string>

class EventListener {
 public:
  virtual ~EventListener() = default;
  virtual void mouseDragStart() = 0;
  virtual void mouseDragEnd() = 0;
  virtual void mouseDrag(int xrel, int yrel) = 0;
  virtual void mouseMotion(int x, int y) = 0;
  virtual void mouseWheel(int x, int y) = 0;
  virtual void mouseClick() = 0;
  virtual void keyPressed(std::string key) = 0;
  virtual void advanceFrame() = 0;
  virtual void queueAudio() = 0;
  virtual void refreshDisplay() = 0;

};

#endif  // #ifndef EVENT_LISTENER_H_
