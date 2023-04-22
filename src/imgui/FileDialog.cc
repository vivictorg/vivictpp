// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "imgui/FileDialog.hh"

std::vector<vivictpp::imgui::Action> vivictpp::imgui::FileDialog::draw() {
  std::vector<Action> actions;
  if (fileDialog.Display("ChooseFileDlgKey", 0, {400,300}))
  {
    // action if OK
    if (fileDialog.IsOk())
    {
      std::string filePathName = fileDialog.GetFilePathName();
      folder = fileDialog.GetCurrentPath();
      ActionType actionType = leftRight == LeftRight::Left ? ActionType::OpenFileLeft
                              : ActionType::OpenFileRight;
      actions.push_back({actionType, 0, {0,0}, filePathName});
    }

    // close
    fileDialog.Close();
  }
  return actions;
}
