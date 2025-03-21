// SPDX-FileCopyrightText: 20235Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIVICTPP_IMGUI_QUALITYFILEDIALOG_HH_
#define VIVICTPP_IMGUI_QUALITYFILEDIALOG_HH_

#include "Events.hh"
#include "ImGuiFileDialog.h"
#include "imgui/FileDialog.hh"
#include "libav/Utils.hh"
#include "platform_folders.h"

namespace vivictpp::imgui {

class QualityFileDialog {
private:
  ImGuiFileDialog fileDialog;
  LeftRight leftRight{LeftRight::Left};
  std::string folder{sago::getVideoFolder()};

private:
  void openDialog(std::string text, std::string currentFile);

public:
  QualityFileDialog() = default;

  void openLeft(std::string currentFile = "");

  void openRight(std::string currentFile = "");

  std::vector<Action> draw();
};

} // namespace vivictpp::imgui

#endif /* VIVICTPP_IMGUI_QUALITYFILEDIALOG_HH_ */
