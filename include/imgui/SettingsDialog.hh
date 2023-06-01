// SPDX-FileCopyrightText: 2023 Gustav Grusell (gustav.grusell@gmail.com)
//
// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef VIVICTPP_IMGUI_SETTINGSDIALOG_HH_
#define VIVICTPP_IMGUI_SETTINGSDIALOG_HH_

#include "Events.hh"
#include "ui/DisplayState.hh"
#include <vector>
#include <string>
#include "Settings.hh"

struct HwAccelStatus {
  std::string name;
  bool enabled;
};


namespace vivictpp::imgui {

class SettingsDialog {
private:
  vivictpp::Settings settings;
  vivictpp::Settings modifiedSettings;
  //bool disableFontAutoScaling{false};
  std::vector<std::string> hwAccelFormats;
  std::vector<HwAccelStatus> hwAccelStatuses;
//  std::vector<std::string> selectedHwAccelFormats;
//  std::vector<std::string> selectableHwAccelFormats;
  float textWidth;
  std::string selected;
private:
  void initHwAccelStatuses();
public:
  SettingsDialog(Settings settings);
  std::vector<Action> draw(ui::DisplayState &displayState);
};

}


#endif /* VIVICTPP_IMGUI_SETTINGSDIALOG_HH_ */
