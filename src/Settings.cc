// SPDX-FileCopyrightText: 2023 Gustav Grusell (gustav.grusell@gmail.com)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "Settings.hh"
#include "fmt/core.h"
#include "platform_folders.h"
#include "toml.hpp"
#include <fstream>
#include <iostream>
#include <map>

std::filesystem::path getSettingsFilePath() {
  return std::filesystem::path(
             fmt::format("{}/vivictpp/vivictpp.toml", sago::getConfigHome()))
      .make_preferred();
}

toml::array toTomlArray(std::vector<std::string> v) {
  toml::array arr;
  for (auto &str : v) {
    arr.push_back(str);
  }
  return arr;
}

toml::array toTomlArray(std::map<std::string, std::string> m) {
  toml::array arr;
  for (auto &e : m) {
    arr.push_back(e.first + "=" + e.second);
  }
  return arr;
}

std::vector<std::string> toVector(const toml::array *arr) {
  std::vector<std::string> result;
  if (!arr)
    return result;
  for (size_t i = 0; i < arr->size(); i++) {
    result.push_back((*arr)[i].as_string()->get());
  }
  return result;
}

std::map<std::string, std::string> toMap(const toml::table *tbl) {
  std::map<std::string, std::string> result;
  if (!tbl)
    return result;
  for (auto it = tbl->begin(); it != tbl->end(); it++) {
    result[std::string(it->first.str())] = it->second.as_string()->get();
  }
  return result;
}

vivictpp::Settings vivictpp::loadSettings() {
  auto filePath = getSettingsFilePath();
  return loadSettings(filePath);
}

void loadInt(int &target, const toml::table &table, const std::string &path) {
  if (table.at_path(path).is_integer()) {
    target = table.at_path(path).as_integer()->get();
  }
}

void loadBool(bool &target, const toml::table &table, const std::string &path) {
  if (table.at_path(path).is_boolean()) {
    target = table.at_path(path).as_boolean()->get();
  }
}

void loadVector(std::vector<std::string> &target, const toml::table &table,
                const std::string &path) {
  if (table.at_path(path).is_array()) {
    target = toVector(table.at_path(path).as_array());
  }
}

void loadString(std::string &target, const toml::table &table,
                const std::string &path) {
  if (table.at_path(path).is_string()) {
    target = table.at_path(path).as_string()->get();
  }
}

void loadMap(std::map<std::string, std::string> &target,
             const toml::table &table, const std::string &path) {
  if (table.at_path(path).is_table()) {
    target = toMap(table.at_path(path).as_table());
  }
}

vivictpp::Settings vivictpp::loadSettings(std::filesystem::path filePath) {
  if (!std::filesystem::exists(filePath)) {
    return {};
  }

  try {
    toml::table toml = toml::parse_file(filePath.string());
    Settings settings;
    loadInt(settings.baseFontSize, toml, "fontsettings.basefontsize");
    loadBool(settings.disableFontAutoScaling, toml,
             "fontsettings.disableautoscaling");
    loadVector(settings.hwAccels, toml, "decoding.enabledHwAccels");
    loadVector(settings.preferredDecoders, toml, "decoding.preferredDecoders");
    loadInt(settings.logBufferSize, toml, "logsettings.logbuffersize");
    loadBool(settings.logToFile, toml, "logsettings.logtofile");
    loadString(settings.logFile, toml, "logsettings.logfile");
    loadMap(settings.logLevels, toml, "loglevels");
    loadBool(settings.autoloadMetrics, toml, "metrics.autoload");
    return settings;

  } catch (const toml::parse_error &err) {
    // TODO: Add logging
    return {};
  }
}

toml::table settingsToToml(const vivictpp::Settings &settings) {
  toml::table tbl;
  toml::table decoding;
  toml::table fontSettings;
  toml::table logSettings;
  toml::table logLevels;
  toml::table metricSettings;
  fontSettings.insert("basefontsize", settings.baseFontSize);
  fontSettings.insert("disableautoscaling", settings.disableFontAutoScaling);
  decoding.insert("enabledHwAccels", toTomlArray(settings.hwAccels));
  decoding.insert("preferredDecoders", toTomlArray(settings.preferredDecoders));

  logSettings.insert("logbuffersize", settings.logBufferSize);
  logSettings.insert("logtofile", settings.logToFile);
  logSettings.insert("logfile", settings.logFile);
  for (auto e : settings.logLevels) {
    logLevels.insert(e.first, e.second);
  }
  metricSettings.insert("autoload", settings.autoloadMetrics);
  tbl.insert("fontsettings", fontSettings);
  tbl.insert("decoding", decoding);
  tbl.insert("logsettings", logSettings);
  tbl.insert("loglevels", logLevels);
  tbl.insert("metrics", metricSettings);
  return tbl;
}

void vivictpp::saveSettings(const vivictpp::Settings &settings) {
  std::ofstream settingsFile;
  settingsFile.open(getSettingsFilePath());
  settingsFile << settingsToToml(settings) << std::endl;
  settingsFile.close();
}

bool vivictpp::operator==(const vivictpp::Settings &lhs,
                          const vivictpp::Settings &rhs) {
  return lhs.baseFontSize == rhs.baseFontSize &&
         lhs.disableFontAutoScaling == rhs.disableFontAutoScaling &&
         lhs.hwAccels == rhs.hwAccels &&
         lhs.preferredDecoders == rhs.preferredDecoders &&
         lhs.logBufferSize == rhs.logBufferSize &&
         lhs.logToFile == rhs.logToFile && lhs.logFile == rhs.logFile &&
         lhs.logLevels == rhs.logLevels;
}
