// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#define SDL_MAIN_HANDLED

#include <fstream>
#include <iostream>
#include <vector>
#include <string>

#include "spdlog/spdlog.h"

#include "OptParser.hh"
#include "SourceConfig.hh"

#include "imgui/VivictPPImGui.hh"
#include "Settings.hh"
#include "VivictPPConfig.hh"

#ifdef _WIN32
#include <windows.h>

int APIENTRY WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR cmdLine,
                   int cmdShow) {
    VivictPPConfig vivictppConfig
#else
int main(int argc, char **argv) {
  try {
    vivictpp::OptParser optParser;
    if (!optParser.parseOptions(argc, argv)) {
      return optParser.exitCode;
    }

    VivictPPConfig vivictPPConfig = optParser.vivictPPConfig;
#endif
    vivictPPConfig.applySettings(vivictpp::loadSettings());
    vivictpp::logging::initializeLogging(vivictPPConfig.settings);


    for (auto sourceConfig : vivictPPConfig.sourceConfigs) {
        spdlog::debug("Source: path={} filters={}", sourceConfig.path, sourceConfig.filter);
    }

    vivictpp::imgui::VivictPPImGui vivictPPImGui(vivictPPConfig);
    vivictPPImGui.run();
  } catch (const std::exception &e) {
    std::cerr << "Vivict++ had an unexpected error: " << e.what() << std::endl;
    return 1;
  }
}
