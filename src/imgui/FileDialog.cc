// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "imgui/FileDialog.hh"
#include "imgui.h"
#include "imgui/WidgetUtils.hh"
#include <algorithm>

void vivictpp::imgui::FileDialog::optionsPane() {
  ImGui::Text("Hardware acceleration");
  vivictpp::imgui::comboBox("##Hardware acceleration", hwAccelOptions,
                            currentHwAccelOption);
  ImGui::Dummy({1, 10});
  ImGui::Text("Preferred decoder");
  vivictpp::imgui::comboBox("##Decoder", preferredDecoderOptions,
                            currentDecoderOption);
  ImGui::Dummy({1, 10});
  ImGui::Text("Filter");
  ImGui::InputText("##filterinput", filterStr, IM_ARRAYSIZE(filterStr));
  ImGui::Dummy({1, 10});
  ImGui::Text("Format options");
  ImGui::InputText("##formatoptions", formatOptionsStr,
                   IM_ARRAYSIZE(formatOptionsStr));
}

void vivictpp::imgui::FileDialog::openLeft(std::string currentFile) {
  this->leftRight = LeftRight::Left;
  openDialog("Choose Left Source File", currentFile);
}

void vivictpp::imgui::FileDialog::openRight(std::string currentFile) {
  this->leftRight = LeftRight::Right;
  openDialog("Choose Right Source File", currentFile);
}

void vivictpp::imgui::FileDialog::openDialog(std::string text,
                                             std::string currentFile) {
  auto optionsPaneCallback = [this](const char *vFilter,
                                    IGFDUserDatas vUserDatas,
                                    bool *vCantContinue) {
    (void)vFilter;
    (void)vUserDatas;
    (void)vCantContinue;
    this->optionsPane();
  };
  std::string folder =
      currentFile.empty()
          ? this->folder + "/"
          : std::filesystem::path(currentFile).parent_path().string();
  fileDialog.OpenDialog("ChooseFileDlgKey", text.c_str(), ".*", folder, "",
                        optionsPaneCallback, 350, 1, nullptr);
}

SourceConfig vivictpp::imgui::FileDialog::getSourceConfig() {
  std::vector<std::string> hwAccels;
  if (selectedHwAccel() == "auto") {
    hwAccels = settings.hwAccels;
  } else if (selectedHwAccel() != "none") {
    hwAccels.push_back(selectedHwAccel());
  }
  std::vector<std::string> preferredDecoders;
  if (selectedDecoder() == "auto") {
    preferredDecoders = settings.preferredDecoders;
  } else {
    preferredDecoders.push_back(selectedDecoder());
  }
  SourceConfig sourceConfig = {fileDialog.GetFilePathName(), hwAccels, preferredDecoders,
                               filter(), formatOptions()};
  return sourceConfig;
}

std::vector<vivictpp::imgui::Action> vivictpp::imgui::FileDialog::draw() {
  std::vector<Action> actions;
  if (fileDialog.Display("ChooseFileDlgKey", 0, {400, 300})) {
    // action if OK
    if (fileDialog.IsOk()) {
      folder = fileDialog.GetCurrentPath();
      SourceConfig sourceConfig = getSourceConfig();
      ActionType actionType = leftRight == LeftRight::Left
                                  ? ActionType::OpenFileLeft
                                  : ActionType::OpenFileRight;
      actions.push_back(Action(actionType, sourceConfig));
    }

    // close
    fileDialog.Close();
  }
  return actions;
}
