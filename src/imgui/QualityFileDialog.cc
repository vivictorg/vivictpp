// SPDX-FileCopyrightText: 2025 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "imgui/QualityFileDialog.hh"
#include "imgui.h"
#include "imgui/WidgetUtils.hh"
#include <algorithm>

void vivictpp::imgui::QualityFileDialog::openLeft(std::string currentFile) {
  this->leftRight = LeftRight::Left;
  openDialog("Choose Left Source File", currentFile);
}

void vivictpp::imgui::QualityFileDialog::openRight(std::string currentFile) {
  this->leftRight = LeftRight::Right;
  openDialog("Choose Right Source File", currentFile);
}

void vivictpp::imgui::QualityFileDialog::openDialog(std::string text,
                                                    std::string currentFile) {
  std::string folder =
      currentFile.empty()
          ? this->folder + "/"
          : std::filesystem::path(currentFile).parent_path().string();
  fileDialog.OpenDialog("ChooseQualityFileDlgKey", text.c_str(), ".json,.csv",
                        folder, "", nullptr, 0, 1, nullptr);
}

std::vector<vivictpp::imgui::Action>
vivictpp::imgui::QualityFileDialog::draw() {
  std::vector<Action> actions;
  if (fileDialog.Display("ChooseQualityFileDlgKey", 0, {400, 300})) {
    // action if OK
    if (fileDialog.IsOk()) {
      std::string filePathName = fileDialog.GetFilePathName();
      folder = fileDialog.GetCurrentPath();
      ActionType actionType = leftRight == LeftRight::Left
                                  ? ActionType::OpenQualityFileLeft
                                  : ActionType::OpenQualityFileRight;
      actions.push_back({actionType, 0, {0, 0}, filePathName});
    }

    // close
    fileDialog.Close();
  }
  return actions;
}
