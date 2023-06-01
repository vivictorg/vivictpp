// SPDX-FileCopyrightText: 2023 Gustav Grusell (gustav.grusell@gmail.com)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "Settings.hh"
#include "toml.hpp"
#include "platform_folders.h"
#include "fmt/core.h"
#include <filesystem>
#include <iostream>
#include <fstream>

std::filesystem::path getSettingsFilePath() {
  return std::filesystem::path(fmt::format("{}/vivictpp/vivictpp.toml", sago::getConfigHome())).make_preferred();
}

toml::array toTomlArray(std::vector<std::string> v) {
  toml::array arr;
  for (auto &str : v) {
    arr.push_back(str);
  }
  return arr;
}

std::vector<std::string> toVector(toml::array *arr) {
  std::vector<std::string> result;
  if (!arr) return result;
  for (size_t i = 0; i < arr->size(); i++) {
    result.push_back((*arr)[i].as_string()->get());
  }
  return result;
}

vivictpp::Settings vivictpp::loadSettings() {
  auto filePath = getSettingsFilePath();
  if (!std::filesystem::exists(filePath)) {
    return {};
  }

  try {
    toml::table toml = toml::parse_file(filePath.string());
    Settings settings;
    settings.baseFontSize = toml["fontsettings"]["basefontsize"].as_integer()->get();
    settings.disableFontAutoScaling = toml["fontsettings"]["disableautoscaling"].as_boolean()->get();
    settings.hwAccels = toVector(toml["decoding"]["enabledHwAccels"].as_array());
    settings.preferredDecoders = toVector(toml["decoding"]["preferredDecoders"].as_array());
    return settings;

  } catch (const toml::parse_error& err) {
    // TODO: Add logging
    return {};
  }

}

toml::table settingsToToml(const vivictpp::Settings &settings) {
  toml::table tbl;
  toml::table decoding;
  toml::table fontSettings;
  fontSettings.insert("basefontsize", settings.baseFontSize);
  fontSettings.insert("disableautoscaling", settings.disableFontAutoScaling);
  decoding.insert("enabledHwAccels", toTomlArray(settings.hwAccels));
  decoding.insert("preferredDecoders", toTomlArray(settings.preferredDecoders));
  tbl.insert("fontsettings", fontSettings);
  tbl.insert("decoding", decoding);
  return tbl;
}

void vivictpp::saveSettings(const vivictpp::Settings &settings) {
  std::ofstream settingsFile;
  settingsFile.open(getSettingsFilePath());
  settingsFile << settingsToToml(settings) << std::endl;
  settingsFile.close();
}
