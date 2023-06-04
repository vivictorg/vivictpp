// SPDX-FileCopyrightText: 2023 Gustav Grusell (gustav.grusell@gmail.com)
//
// SPDX-License-Identifier: GPL-2.0-or-later


#include "imgui/SettingsDialog.hh"
#include "imgui.h"
#include "imgui/Events.hh"
#include "libav/Utils.hh"
#include "ui/DisplayState.hh"
#include "logging/Logging.hh"
#include "libav/HwAccelUtils.hh"
#include "imgui/Fonts.hh"
#include "imgui/WidgetUtils.hh"

bool vectorHasElement(std::vector<std::string> v, std::string element) {
  return std::find(v.begin(), v.end(), element) != v.end();
}

vivictpp::imgui::SettingsDialog::SettingsDialog(vivictpp::Settings _settings) :
  settings(_settings),
  modifiedSettings(_settings),
  hwAccelFormats(vivictpp::libav::allHwAccelFormats()),
  decoders(vivictpp::libav::allVideoDecoders()) {
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
    ImGui::Separator();
    ImGui::Text("Hardware acceleration");
    ImGui::Indent();
    std::function<void(HwAccelStatus&)> renderHwAccelStatus = [](HwAccelStatus &has) {
      ImGui::Checkbox(has.name.c_str(), &has.enabled);
    };
    prioritizedList("hwaccellist", ImGui::GetContentRegionAvail().x * 0.6f, hwAccelStatuses, renderHwAccelStatus);

    ImGui::Unindent();
    ImGui::Separator();
    ImGui::Text("Preferred decoders");
    ImGui::Indent();
    std::function<void(std::string&)> renderDecoder = [](std::string &decoder) {
      ImGui::Text("%s", decoder.c_str());
    };

    prioritizedList("preferredDecoders", ImGui::GetContentRegionAvail().x * 0.6f,
                    modifiedSettings.preferredDecoders, renderDecoder, true);
    std::function<bool(const std::string&)> exclude = [&](const std::string &decoder) -> bool {
      return std::find(modifiedSettings.preferredDecoders.begin(), modifiedSettings.preferredDecoders.end(), decoder) !=
        modifiedSettings.preferredDecoders.end();
    };

    static std::string currentDecoder = decoders[0];
    ImGui::Text("---");
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.6f);
    comboBox("##Decoders", decoders, currentDecoder, exclude);
    ImGui::SameLine();
    if (ImGui::Button("add")) {
      modifiedSettings.preferredDecoders.push_back(currentDecoder);
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
