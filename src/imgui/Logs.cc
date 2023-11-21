// SPDX-FileCopyrightText: 2023 Gustav Grusell (gustav.grusell@gmail.com)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "imgui.h"

#include "imgui/Logs.hh"
#include "logging/Logging.hh"

#include <string>

void vivictpp::imgui::showLogs(ui::DisplayState &displayState) {
  if (ImGui::Begin("Logs", &displayState.displayLogs, ImGuiWindowFlags_NoCollapse)) {
      for (std::string row : vivictpp::logging::getMessages()) {
            ImGui::TextUnformatted(row.c_str());
      }
      ImGui::End();
  }
}
