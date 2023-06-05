// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "imgui/FileDialog.hh"
#include "imgui.h"
#include <algorithm>
#include "imgui/WidgetUtils.hh"

void vivictpp::imgui::FileDialog::optionsPane() {
    ImGui::Text("Hardware acceleration");
    vivictpp::imgui::comboBox("##Hardware acceleration", hwAccelOptions, currentHwAccelOption);
    ImGui::Dummy({1,10});
    ImGui::Text("Preferred decoder");
    vivictpp::imgui::comboBox("##Decoder", preferredDecoderOptions, currentDecoderOption);
    ImGui::Dummy({1,10});
    ImGui::Text("Filter");
    ImGui::InputText("##filterinput", filterStr, IM_ARRAYSIZE(filterStr));
    ImGui::Dummy({1,10});
    ImGui::Text("Format options");
    ImGui::InputText("##formatoptions", formatOptionsStr, IM_ARRAYSIZE(formatOptionsStr));
}

void vivictpp::imgui::FileDialog::openLeft() {
  this->leftRight = LeftRight::Left;
  openDialog( "Choose Left Source File");
}

void vivictpp::imgui::FileDialog::openRight() {
  this->leftRight = LeftRight::Right;
  openDialog("Choose Right Source File");
}

void vivictpp::imgui::FileDialog::openDialog(std::string text) {
  auto optionsPaneCallback = [this](const char *vFilter, IGFDUserDatas vUserDatas, bool *vCantContinue) {
                          (void) vFilter;
                          (void) vUserDatas;
                          (void) vCantContinue;
                          this->optionsPane();
  };
  fileDialog.OpenDialog("ChooseFileDlgKey", text.c_str(), ".*", folder + "/", "",
                        optionsPaneCallback, 350, 1, nullptr);
}

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
