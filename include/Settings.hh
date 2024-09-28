// SPDX-FileCopyrightText: 2023 Gustav Grusell (gustav.grusell@gmail.com)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIVICTPP_SETTINGS_HH_
#define VIVICTPP_SETTINGS_HH_

#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace vivictpp {

struct Settings {
  bool disableFontAutoScaling{false};
  int baseFontSize{13};
  std::vector<std::string> hwAccels{{}};
  std::vector<std::string> preferredDecoders{{}};
  int logBufferSize{128};
  bool logToFile{false};
  std::string logFile;
  std::map<std::string, std::string> logLevels{{"default", "info"}};
};

Settings loadSettings(std::filesystem::path filePath);

Settings loadSettings();

void saveSettings(const Settings &settings);

bool operator==(const Settings &lhs, const Settings &rhs);

} // namespace vivictpp

#endif /* VIVICTPP_SETTINGS_HH_ */
