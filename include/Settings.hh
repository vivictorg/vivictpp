// SPDX-FileCopyrightText: 2023 Gustav Grusell (gustav.grusell@gmail.com)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIVICTPP_SETTINGS_HH_
#define VIVICTPP_SETTINGS_HH_

#include <vector>
#include <string>

namespace vivictpp {

struct Settings {
  bool disableFontAutoScaling;
  int baseFontSize;
  std::vector<std::string> hwAccels;
  std::vector<std::string> preferredDecoders;
};

Settings loadSettings();

void saveSettings(const Settings &settings);

}

#endif /* VIVICTPP_SETTINGS_HH_ */
