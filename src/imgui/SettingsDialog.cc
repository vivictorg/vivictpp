// SPDX-FileCopyrightText: 2023 Gustav Grusell (gustav.grusell@gmail.com)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "imgui/SettingsDialog.hh"
#include "imgui.h"
#include "imgui/Events.hh"
#include "imgui/Fonts.hh"
#include "imgui/WidgetUtils.hh"
#include "libav/HwAccelUtils.hh"
#include "libav/Utils.hh"
#include "logging/Logging.hh"
#include "ui/DisplayState.hh"

std::string longestString(const std::vector<std::string> &strs);
std::vector<std::string> getSelectableLogLevels();

static const std::vector<std::string> selectableLogLevels =
    getSelectableLogLevels();

static const std::string longestLoggerName =
    longestString(vivictpp::logging::getLoggers());

std::string longestString(const std::vector<std::string> &strs) {
  if (strs.empty())
    return "";
  std::string longest = strs[0];
  for (size_t i = 1; i < strs.size(); i++) {
    if (strs[i].length() > longest.length()) {
      longest = strs[i];
    }
  }
  return longest;
}

std::vector<std::string> getSelectableLogLevels() {
  std::vector<std::string> levels;
  levels.push_back("(default)");
  for (auto l : vivictpp::logging::getLogLevels()) {
    levels.push_back(l.first);
  }
  return levels;
}

void copyAndNullTerminate(const std::string &str, char *buf, size_t bufSize) {
  str.copy(buf, bufSize - 1);
  buf[std::min(str.length(), bufSize - 1)] = '\0';
}

bool vectorHasElement(std::vector<std::string> v, std::string element) {
  return std::find(v.begin(), v.end(), element) != v.end();
}

vivictpp::imgui::SettingsDialog::SettingsDialog(vivictpp::Settings _settings)
    : settings(_settings), modifiedSettings(_settings),
      hwAccelFormats(vivictpp::libav::allHwAccelFormats()),
      decoders(vivictpp::libav::allVideoDecoders()) {
  copyAndNullTerminate(settings.logFile, logFileStr, 512);
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

std::vector<vivictpp::imgui::Action> vivictpp::imgui::SettingsDialog::draw(
    vivictpp::ui::DisplayState &displayState) {
  fontSettingsUpdated = false;

  if (ImGui::Begin("Settings", &displayState.displaySettingsDialog)) {
    ImGui::Text("Font size");
    ImGui::Indent();
    bool &disableFontAutoScaling = modifiedSettings.disableFontAutoScaling;
    ImGui::Checkbox("Disable font autoscaling", &disableFontAutoScaling);
    ImGui::Text("Base font size");
    ImGui::SameLine();
    ImVec2 ts = ImGui::CalcTextSize("000");
    // Use the fact that the buttons are square so button width equals text
    // height
    ImGui::SetNextItemWidth(ts.x + 3 * ImGui::GetFrameHeight());
    if (ImGui::InputInt("##Base font size input",
                        &modifiedSettings.baseFontSize)) {
      fontSettingsUpdated = true;
      modifiedSettings.baseFontSize =
          std::clamp(modifiedSettings.baseFontSize, 8, 64);
    }
    ImGui::Unindent();
    ImGui::Separator();
    ImGui::Text("Hardware acceleration");
    ImGui::Indent();
    std::function<void(HwAccelStatus &)> renderHwAccelStatus =
        [](HwAccelStatus &has) {
          ImGui::Checkbox(has.name.c_str(), &has.enabled);
        };
    prioritizedList("hwaccellist", ImGui::GetContentRegionAvail().x * 0.6f,
                    hwAccelStatuses, renderHwAccelStatus);

    ImGui::Unindent();
    ImGui::Separator();
    ImGui::Text("Preferred decoders");
    ImGui::Indent();
    std::function<void(std::string &)> renderDecoder =
        [](std::string &decoder) { ImGui::Text("%s", decoder.c_str()); };

    prioritizedList("preferredDecoders",
                    ImGui::GetContentRegionAvail().x * 0.6f,
                    modifiedSettings.preferredDecoders, renderDecoder, true);
    std::function<bool(const std::string &)> exclude =
        [&](const std::string &decoder) -> bool {
      return std::find(modifiedSettings.preferredDecoders.begin(),
                       modifiedSettings.preferredDecoders.end(),
                       decoder) != modifiedSettings.preferredDecoders.end();
    };

    static std::string currentDecoder = decoders[0];
    ImGui::Text("---");
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.6f);
    comboBox("##Decoders", decoders, currentDecoder, exclude);
    ImGui::SameLine();
    if (ImGui::Button("add")) {
      modifiedSettings.preferredDecoders.push_back(currentDecoder);
    }
    ImGui::Unindent();

    ImGui::Separator();
    ImGui::Text("Logging");
    ImGui::Indent();
    ImGui::Text("Log buffer size");
    ts = ImGui::CalcTextSize("0000");
    // Use the fact that the buttons are square so button width equals text
    // height
    ImGui::SetNextItemWidth(ts.x + 3 * ImGui::GetFrameHeight());
    if (ImGui::InputInt("## Log buffer size input",
                        &modifiedSettings.logBufferSize)) {
      // fontSettingsUpdated = true;
      modifiedSettings.logBufferSize =
          std::clamp(modifiedSettings.logBufferSize, 1, 1024);
    }
    bool &logToFile = modifiedSettings.logToFile;
    ImGui::Checkbox("Log to file", &logToFile);
    ImGui::SameLine();

    ImGui::BeginDisabled(!logToFile);
    ImGui::InputText("### Log file", logFileStr, IM_ARRAYSIZE(logFileStr));
    ImGui::EndDisabled();

    if (ImGui::CollapsingHeader("Loglevels")) {
      ts = ImGui::CalcTextSize(longestLoggerName.c_str());
      int i = 0;
      for (auto logger : vivictpp::logging::getLoggers()) {
        if (modifiedSettings.logLevels.find(logger) ==
            modifiedSettings.logLevels.end()) {
          if (logger == vivictpp::logging::LIBAV_LOGGER_NAME) {
            modifiedSettings.logLevels[logger] = "warning";
          } else {
            modifiedSettings.logLevels[logger] = "(default)";
          }
        }
        std::string &selectedLevel = modifiedSettings.logLevels[logger];
        ImGui::Text("%s", logger.c_str());
        ImGui::SameLine(ts.x + 100);
        ImGui::PushID(i++);
        ImGui::SetNextItemWidth(200.);
        if (ImGui::BeginCombo("### Defaultlogger", selectedLevel.c_str())) {
          int j = 0;
          for (auto level : selectableLogLevels) {
            ImGui::PushID(j);
            const bool isSelected = selectedLevel == level;
            if (ImGui::Selectable(level.c_str(), isSelected)) {
              selectedLevel = level;
            }
            if (isSelected) {
              ImGui::SetItemDefaultFocus();
            }
            ImGui::PopID();
          }
          ImGui::EndCombo();
        }
        ImGui::PopID();
      }
    }
  }

  std::vector<vivictpp::imgui::Action> actions;
  ImGui::Dummy({20, 20});
  if (ImGui::Button("Cancel")) {
    fontSettingsUpdated =
        modifiedSettings.baseFontSize != settings.baseFontSize ||
        modifiedSettings.disableFontAutoScaling !=
            settings.disableFontAutoScaling;
    modifiedSettings = settings;
    copyAndNullTerminate(settings.logFile, logFileStr, 512);
    initHwAccelStatuses();
    displayState.displaySettingsDialog = false;
  }
  ImGui::SameLine();
  ImGui::Dummy({50, 20});
  ImGui::SameLine();
  if (ImGui::Button("OK")) {
    // Save settings
    modifiedSettings.logFile = std::string(logFileStr);
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
