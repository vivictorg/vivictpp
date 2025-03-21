// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIVICTPP_IMGUI_EVENTS_HH_
#define VIVICTPP_IMGUI_EVENTS_HH_

#include "imgui.h"
#include "time/Time.hh"
#include <memory>
#include <string>

namespace vivictpp::imgui {

class Event {
public:
  virtual ~Event() = default;
};

class Quit : public Event {};

class WindowSizeChange : public Event {};

class MouseMotion : public Event {
public:
  MouseMotion(int x, int y) : x(x), y(y) {}
  int x;
  int y;
};

class KeyEvent : public Event {
public:
  KeyEvent(std::string keyName, bool shift, bool ctrl, bool alt)
      : keyName(keyName), shift(shift), ctrl(ctrl), alt(alt){};
  std::string keyName;
  bool shift{false};
  bool ctrl{false};
  bool alt{false};
  bool noModifiers() const { return !shift && !ctrl && !alt; }
  bool isShift() const { return shift && !ctrl && !alt; }
  bool isCtrl() const { return !shift && ctrl && !alt; }
  bool isCtrlShift() const { return shift && ctrl && !alt; }
  bool isCtrlAlt() const { return !shift && ctrl && alt; }
};

enum ActionType {
  NoAction,
  ActionQuit,
  PlayPause,
  ZoomIn,
  ZoomOut,
  ZoomReset,
  Seek,
  SeekRelative,
  StepForward,
  StepBackward,
  FrameOffsetIncrease,
  FrameOffsetDecrease,
  ToggleFullscreen,
  ToggleDisplayTime,
  ToggleDisplayMetadata,
  ToggleDisplayPlot,
  ToggleFitToScreen,
  PlaybackSpeedIncrease,
  PlaybackSpeedDecrease,
  ToggleImGuiDemo,
  Scroll,
  OpenFileLeft,
  OpenFileRight,
  ShowFileDialogLeft,
  ShowFileDialogRight,
  ShowHelp,
  ShowAbout,
  ShowSettingsDialog,
  UpdateSettings,
  ShowLogs,
  ShowQualityFileDialogLeft,
  ShowQualityFileDialogRight,
  OpenQualityFileLeft,
  OpenQualityFileRight,
};

struct Action {
  Action(ActionType type, vivictpp::time::Time seek = 0, ImVec2 scroll = {0, 0},
         std::string file = "")
      : type(type), seek(seek), scroll(scroll), file(file){};
  ActionType type;
  vivictpp::time::Time seek;
  ImVec2 scroll;
  std::string file;
};

} // namespace vivictpp::imgui

#endif /* VIVICTPP_IMGUI_EVENTS_HH_ */
