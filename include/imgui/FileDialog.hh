// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIVICTPP_IMGUI_FILEDIALOG_HH_
#define VIVICTPP_IMGUI_FILEDIALOG_HH_

#include "ImGuiFileDialog.h"
#include "Events.hh"
#include "platform_folders.h"

namespace vivictpp::imgui {

enum class LeftRight {Left, Right};

class FileDialog {
private:
  ImGuiFileDialog fileDialog;
  LeftRight leftRight{LeftRight::Left};
  std::string folder{sago::getVideoFolder()};

public:
  FileDialog() {}

  void openLeft() {
    this->leftRight = LeftRight::Left;
    fileDialog.OpenDialog("ChooseFileDlgKey", "Choose Left Source File", ".*", folder + "/");
  }

  void openRight() {
    this->leftRight = LeftRight::Right;
    fileDialog.OpenDialog("ChooseFileDlgKey", "Choose Right Source File", ".*", folder + "/");
  }

  std::vector<Action> draw();

};

}  // namespace vivictpp::imgui

#endif /* VIVICTPP_IMGUI_FILEDIALOG_HH_ */
