// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIVICTPP_IMGUI_EVENTS_HH_
#define VIVICTPP_IMGUI_EVENTS_HH_

#include "SourceConfig.hh"
#include "imgui.h"
#include "time/Time.hh"
#include <memory>
#include <string>
#include <variant>

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
  CycleABLoop,
};

// Action data types
struct NoData {};
struct SeekData {
  vivictpp::time::Time time;
};
struct ScrollData {
  ImVec2 delta;
};
struct FileData {
  std::string path;
};
struct SourceConfigData {
  SourceConfig config;
};

struct Action {
  ActionType type;
  std::variant<NoData, SeekData, ScrollData, FileData, SourceConfigData> data;

  // Constructors for different action types
  Action(ActionType t) : type(t), data(NoData{}) {}

  Action(ActionType t, vivictpp::time::Time seekTime)
      : type(t), data(SeekData{seekTime}) {}

  Action(ActionType t, ImVec2 scrollDelta)
      : type(t), data(ScrollData{scrollDelta}) {}

  Action(ActionType t, std::string filePath)
      : type(t), data(FileData{std::move(filePath)}) {}

  Action(ActionType t, SourceConfig sourceConfig)
      : type(t), data(SourceConfigData{std::move(sourceConfig)}) {}

  // Helper methods for type-safe access
  const SeekData &getSeekData() const { return std::get<SeekData>(data); }

  const ScrollData &getScrollData() const {
    return std::get<ScrollData>(data);
  }

  const FileData &getFileData() const { return std::get<FileData>(data); }

  const SourceConfigData &getSourceConfigData() const {
    return std::get<SourceConfigData>(data);
  }
};

} // namespace vivictpp::imgui

#endif /* VIVICTPP_IMGUI_EVENTS_HH_ */
