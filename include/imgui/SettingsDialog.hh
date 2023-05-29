// SPDX-FileCopyrightText: 2023 Gustav Grusell (gustav.grusell@gmail.com)
//
// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef VIVICTPP_IMGUI_SETTINGSDIALOG_HH_
#define VIVICTPP_IMGUI_SETTINGSDIALOG_HH_

#include "Events.hh"
#include "ui/DisplayState.hh"
#include <vector>
#include <string>

struct Settings {
  bool disableFontAutoScaling;
  int baseFontSize{13};
  std::vector<std::string> hwAccels;
  std::vector<std::string> preferredDecoders;
};

namespace vivictpp::imgui {

class SettingsDialog {
private:
  Settings settings;
  bool disableFontAutoScaling{false};
  std::vector<std::string> hwAccelFormats;
  std::vector<std::string> selectedHwAccelFormats;
  std::vector<std::string> selectableHwAccelFormats;
  float textWidth;
  std::string selected;
public:
  SettingsDialog();
  std::vector<Action> draw(ui::DisplayState &displayState);
};

}


#endif /* VIVICTPP_IMGUI_SETTINGSDIALOG_HH_ */
