// SPDX-FileCopyrightText: 2023 Gustav Grusell (gustav.grusell@gmail.com)
//
// SPDX-License-Identifier: GPL-2.0-or-later


#include "imgui/SettingsDialog.hh"
#include "imgui.h"
#include "imgui/Events.hh"
#include "ui/DisplayState.hh"
#include "logging/Logging.hh"
#include "libav/HwAccelUtils.hh"
#include "imgui/Fonts.hh"

bool vectorHasElement(std::vector<std::string> v, std::string element) {
  return std::find(v.begin(), v.end(), element) != v.end();
}

vivictpp::imgui::SettingsDialog::SettingsDialog(vivictpp::Settings _settings) :
  settings(_settings),
  modifiedSettings(_settings),
  hwAccelFormats(vivictpp::libav::allHwAccelFormats()) {
  initHwAccelStatuses();
}

void vivictpp::imgui::SettingsDialog::initHwAccelStatuses() {
  hwAccelStatuses.clear();
  for (auto &name : settings.hwAccels) {
    if (vectorHasElement(hwAccelFormats, name)) {
      hwAccelStatuses.push_back({name, true});
    }
  }
  for (auto &name : hwAccelFormats) {
    if (!vectorHasElement(settings.hwAccels, name)) {
      hwAccelStatuses.push_back({name, false});
    }
  }
}

std::vector<vivictpp::imgui::Action> vivictpp::imgui::SettingsDialog::draw(vivictpp::ui::DisplayState &displayState) {
  fontSettingsUpdated = false;
//  textWidth = 0.0f;
  ImVec2 textSize = {0.0f, 0.0f};
  for (const auto &name: hwAccelFormats) {
    ImVec2 ts = ImGui::CalcTextSize(name.c_str());
    textSize.x = std::max(textSize.x, ts.x);
    textSize.y = std::max(textSize.y, ts.y);
  }
//  bool b2 = disableFontAutoScaling;
  if (ImGui::Begin("Settings", &displayState.displaySettingsDialog)) {
    ImGui::Text("Font size");
    ImGui::Indent();
    bool &disableFontAutoScaling = modifiedSettings.disableFontAutoScaling;
    ImGui::Checkbox("Disable font autoscaling", &disableFontAutoScaling);
    ImGui::Text("Base font size");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    if (ImGui::InputInt("##Base font size input", &modifiedSettings.baseFontSize)) {
      fontSettingsUpdated = true;
      modifiedSettings.baseFontSize = std::clamp(modifiedSettings.baseFontSize, 8, 64);
    }
    ImGui::Unindent();
    //  disableFontAutoScaling = b2;
    ImGui::Separator();
    ImGui::Text("Hardware acceleration");
    ImGui::Indent();
    ImGui::BeginGroup();
    for (size_t n = 0; n < hwAccelStatuses.size(); n++) {
      HwAccelStatus current = hwAccelStatuses[n];
      bool &myBool = hwAccelStatuses[n].enabled;
//      ImGui::SetNextItemWidth(ImGui::CalcTextSize(current.c_str()).x);
//      ImGui::SetNextItemWidth(width + 50);
      ImGui::Checkbox(current.name.c_str(), &myBool);
    }
    ImGui::EndGroup();
    if (hwAccelStatuses.size() > 1) {
      ImGui::SameLine();
      ImGui::Dummy({50,10});
      ImGui::SameLine();
      ImGui::BeginGroup();
      ImGui::PushItemWidth(8.0f);
      for (size_t n = 0; n < hwAccelStatuses.size(); n++) {
        HwAccelStatus current = hwAccelStatuses[n];
        ImGui::PushID(n);
        if (n == 0) {
          ImGui::BeginDisabled();
        }
        if (ImGui::ArrowButton("##moveup", ImGuiDir_Up)) {
          int n_next = n-1;
          hwAccelStatuses[n] = hwAccelStatuses[n_next];
          hwAccelStatuses[n_next] = current;
        }
        if (n == 0) {
          ImGui::EndDisabled();
        }
        ImGui::SameLine();
        if (n == hwAccelStatuses.size() - 1) {
          ImGui::BeginDisabled();
        }
        if (ImGui::ArrowButton("##movedown", ImGuiDir_Down)) {
          int n_next = n+1;
          hwAccelStatuses[n] = hwAccelStatuses[n_next];
          hwAccelStatuses[n_next] = current;
        }
        if (n == hwAccelStatuses.size() - 1) {
          ImGui::EndDisabled();
        }
        ImGui::PopID();
      }
      ImGui::PopItemWidth();
      ImGui::EndGroup();
    }
  }

  std::vector<vivictpp::imgui::Action> actions;
  ImGui::Unindent();
  ImGui::Dummy({20,20});
  if (ImGui::Button("Cancel")) {
    fontSettingsUpdated = modifiedSettings.baseFontSize != settings.baseFontSize ||
                          modifiedSettings.disableFontAutoScaling != settings.disableFontAutoScaling;
    modifiedSettings = settings;
    initHwAccelStatuses();
    displayState.displaySettingsDialog = false;
  }
  ImGui::SameLine();
  ImGui::Dummy({50,20});
    ImGui::SameLine();
  if (ImGui::Button("OK")) {
    // Save settings
    modifiedSettings.hwAccels.clear();
    for (auto &status : hwAccelStatuses) {
      if (status.enabled) {
        modifiedSettings.hwAccels.push_back(status.name);
      }
    }
    settings = modifiedSettings;
    displayState.displaySettingsDialog = false;
    actions.push_back({vivictpp::imgui::ActionType::UpdateSettings});
  }

  ImGui::End();
//  fontSettingsUpdated
  return actions;
}
