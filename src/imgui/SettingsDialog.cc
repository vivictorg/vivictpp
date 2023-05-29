// SPDX-FileCopyrightText: 2023 Gustav Grusell (gustav.grusell@gmail.com)
//
// SPDX-License-Identifier: GPL-2.0-or-later


#include "imgui/SettingsDialog.hh"
#include "imgui.h"
#include "imgui/Events.hh"
#include "ui/DisplayState.hh"
#include "logging/Logging.hh"
#include "libav/HwAccelUtils.hh"

vivictpp::imgui::SettingsDialog::SettingsDialog():
  hwAccelFormats(vivictpp::libav::allHwAccelFormats()),
  selectableHwAccelFormats(hwAccelFormats) {
}

std::vector<vivictpp::imgui::Action> vivictpp::imgui::SettingsDialog::draw(vivictpp::ui::DisplayState &displayState) {
  bool myBool;

  textWidth = 0.0f;
  for (const auto &name: hwAccelFormats) {
    textWidth = std::max(textWidth, ImGui::CalcTextSize(name.c_str()).x);
  }
//  bool b2 = disableFontAutoScaling;
  if (ImGui::Begin("Settings", &displayState.displaySettingsDialog)) {
    ImGui::Checkbox("Disable font autoscaling", &disableFontAutoScaling);
    ImGui::Text("Base font size");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    if (ImGui::InputInt("##Base font size input", &settings.baseFontSize)) {
      settings.baseFontSize = std::clamp(settings.baseFontSize, 8, 64);
    }
    //  disableFontAutoScaling = b2;
    ImGui::Separator();
    ImGui::Text("Hardware acceleration");
    ImGui::Indent();
    float width = textWidth;
    for (int n = 0; n < selectedHwAccelFormats.size(); n++) {
      std::string current = selectedHwAccelFormats[n];
//      ImGui::SetNextItemWidth(ImGui::CalcTextSize(current.c_str()).x);
//      ImGui::SetNextItemWidth(100);
      ImGui::SetNextItemWidth(width);
      ImGui::Text(current.c_str());
            /*
      ImGui::Selectable(current.c_str(), false, 0, {width, 0.0f});
      if (ImGui::IsItemActive() && !ImGui::IsItemHovered()) {
        int n_next = n + (ImGui::GetMouseDragDelta(0).y < 0.f ? -1 : 1);
        if (n_next >= 0 && n_next < selectedHwAccelFormats.size()) {
          //item_names[n] = item_names[n_next];
          selectedHwAccelFormats[n] = selectedHwAccelFormats[n_next];
          selectedHwAccelFormats[n_next] = current;
          ImGui::ResetMouseDragDelta();
        }
      }
      */
      ImGui::SameLine();
      ImGui::PushID(n);
      if (ImGui::ArrowButton("##moveup", ImGuiDir_Up) && n > 0) {
        int n_next = n-1;
        selectedHwAccelFormats[n] = selectedHwAccelFormats[n_next];
        selectedHwAccelFormats[n_next] = current;
      }
      ImGui::SameLine();
      if (ImGui::ArrowButton("##movedown", ImGuiDir_Down) && n < selectableHwAccelFormats.size() - 1) {
        int n_next = n+1;
        selectedHwAccelFormats[n] = selectedHwAccelFormats[n_next];
        selectedHwAccelFormats[n_next] = current;
      }
      ImGui::SameLine();
      if (ImGui::Button("x")) {
        selectableHwAccelFormats.push_back(selectedHwAccelFormats[n]);
        selectedHwAccelFormats.erase(selectedHwAccelFormats.begin() + n);
        n--;
      }
      ImGui::PopID();
    }
    if (selectableHwAccelFormats.size() > 0) {
      ImGui::SetNextItemWidth(width + 20);
      if (ImGui::BeginCombo("##addhwaccel", selected.c_str())) {
        for (auto &a : selectableHwAccelFormats) {
          if (ImGui::Selectable(a.c_str(), false, 0, {width, 0.0f} )) {
            selected = a;
            spdlog::info("Selected: {}", selected);
          }
        }
        ImGui::EndCombo();
      }
      ImGui::SameLine();
      if (ImGui::Button("Add")) {
        selectedHwAccelFormats.push_back(selected);
        selectableHwAccelFormats.erase(std::remove(selectableHwAccelFormats.begin(), selectableHwAccelFormats.end(), selected),
                                       selectableHwAccelFormats.end());
        selected = selectableHwAccelFormats.size() > 0 ? selectableHwAccelFormats[0] : "";
      }
    }
    ImGui::Unindent();
//    for (auto &hwAccel : hwAccelFormats) {
//
//    }
  }
  ImGui::End();
  return std::vector<vivictpp::imgui::Action>();
}
