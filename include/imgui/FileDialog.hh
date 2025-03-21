// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIVICTPP_IMGUI_FILEDIALOG_HH_
#define VIVICTPP_IMGUI_FILEDIALOG_HH_

#include "Events.hh"
#include "ImGuiFileDialog.h"
#include "libav/HwAccelUtils.hh"
#include "libav/Utils.hh"
#include "platform_folders.h"

namespace vivictpp::imgui {

enum class LeftRight { Left, Right };

class FileDialog {
private:
  ImGuiFileDialog fileDialog;
  LeftRight leftRight{LeftRight::Left};
  std::string folder{sago::getVideoFolder()};
  std::vector<std::string> hwAccelOptions;
  std::vector<std::string> preferredDecoderOptions;
  std::string currentHwAccelOption;
  std::string currentDecoderOption;
  char filterStr[512]{'\0'};
  char formatOptionsStr[512]{'\0'};

private:
  void openDialog(std::string text, std::string currentFile);
  void optionsPane();

public:
  FileDialog()
      : hwAccelOptions({"auto", "none"}), preferredDecoderOptions({"auto"}),
        currentHwAccelOption(hwAccelOptions[0]),
        currentDecoderOption(preferredDecoderOptions[0]) {
    for (auto &hwAccel : vivictpp::libav::allHwAccelFormats()) {
      hwAccelOptions.push_back(hwAccel);
    }
    for (auto &decoder : vivictpp::libav::allVideoDecoders()) {
      preferredDecoderOptions.push_back(decoder);
    }
  }

  void openLeft(std::string currentFile = "");

  void openRight(std::string currentFile = "");

  const std::string selectedHwAccel() { return currentHwAccelOption; }

  const std::string selectedDecoder() { return currentDecoderOption; }

  const std::string filter() { return std::string(filterStr); }

  const std::string formatOptions() { return std::string(formatOptionsStr); }

  std::vector<Action> draw();
};

} // namespace vivictpp::imgui

#endif /* VIVICTPP_IMGUI_FILEDIALOG_HH_ */
