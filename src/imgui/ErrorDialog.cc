// SPDX-FileCopyrightText: 2025 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "imgui/ErrorDialog.hh"
#include "imgui.h"
#include <iomanip>
#include <sstream>

namespace vivictpp::imgui {

ErrorDialog::ErrorDialog(vivictpp::ErrorQueue &errorQueue)
    : errorQueue(errorQueue) {}

void ErrorDialog::checkForErrors() {
  if (errorQueue.hasErrors() && !isOpen) {
    shouldOpen = true;
  }
}

bool ErrorDialog::draw() {
  // Open modal if we have new errors
  if (shouldOpen) {
    auto newErrors = errorQueue.getErrors();
    displayedErrors.insert(displayedErrors.end(), newErrors.begin(),
                           newErrors.end());
    ImGui::OpenPopup("Error");
    isOpen = true;
    shouldOpen = false;
  }

  // Center the modal
  ImVec2 center = ImGui::GetMainViewport()->GetCenter();
  ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);

  bool open = true;
  if (ImGui::BeginPopupModal("Error", &open,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::TextWrapped("The following errors occurred:");
    ImGui::Separator();

    // Scrollable region for errors
    ImGui::BeginChild("ErrorList", ImVec2(580, 300), true);

    for (const auto &error : displayedErrors) {
      // Format timestamp
      auto time = std::chrono::system_clock::to_time_t(error.timestamp);
      std::tm tm = *std::localtime(&time);
      std::stringstream ss;
      ss << std::put_time(&tm, "%H:%M:%S");

      // Display error with thread name and timestamp
      ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
      ImGui::TextWrapped("[%s] %s:", ss.str().c_str(),
                         error.threadName.c_str());
      ImGui::PopStyleColor();

      ImGui::TextWrapped("%s", error.message.c_str());
      ImGui::Spacing();
      ImGui::Separator();
    }

    ImGui::EndChild();

    ImGui::Spacing();

    // Button to close
    if (ImGui::Button("OK", ImVec2(120, 0))) {
      displayedErrors.clear();
      isOpen = false;
      ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
  } else {
    // Dialog was closed via X button
    if (isOpen) {
      displayedErrors.clear();
      isOpen = false;
    }
  }

  return isOpen;
}

} // namespace vivictpp::imgui
