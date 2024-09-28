// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIVICTPP_IMGUI_VIVICTPPIMGUI_HH_
#define VIVICTPP_IMGUI_VIVICTPPIMGUI_HH_

#include "Settings.hh"
#include "VideoPlayback.hh"
#include "VideoWindow.hh"
#include "VivictPPConfig.hh"
#include "imgui.h"
#include "imgui/Controls.hh"
#include "imgui/Events.hh"
#include "imgui/FileDialog.hh"
#include "imgui/ImGuiSDL.hh"
#include "imgui/MainMenu.hh"
#include "imgui/SettingsDialog.hh"
#include "imgui/VideoMetadataDisplay.hh"
#include "sdl/SDLUtils.hh"
#include "ui/DisplayState.hh"
#include "ui/VideoTextures.hh"
#include <vector>

namespace vivictpp::imgui {

/*
class TimeDisplay {
public:
  void draw(const ui::DisplayState &displayState);
};
*/

class VivictPPImGui {
private:
  Settings settings;
  ImGuiSDL imGuiSDL;
  VideoPlayback videoPlayback;
  bool done{false};
  ui::DisplayState displayState;
  VideoWindow videoWindow;
  Controls controls;
  int64_t tLastPresent{0};
  FileDialog fileDialog;
  MainMenu mainMenu;
  SettingsDialog settingsDialog;

private:
  Action handleKeyEvent(const KeyEvent &keyEvent);
  std::vector<Action>
  handleEvents(std::vector<std::shared_ptr<vivictpp::imgui::Event>> events);
  void handleActions(std::vector<vivictpp::imgui::Action> actions);
  void openFile(const vivictpp::imgui::Action &action);

public:
  VivictPPImGui(const VivictPPConfig &vivictPPConfig);
  void run();
};

} // namespace vivictpp::imgui

#endif /* VIVICTPP_IMGUI_VIVICTPPIMGUI_HH_ */
