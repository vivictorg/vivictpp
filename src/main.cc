// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#define SDL_MAIN_HANDLED

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "spdlog/spdlog.h"

#include "OptParser.hh"
#include "SourceConfig.hh"

#include "Settings.hh"
#include "VivictPPConfig.hh"
#include "imgui/VivictPPImGui.hh"

#ifdef _WIN32
#include <windows.h>

namespace utf8 {
char **get_argv(int *argc);
void free_argv(int argc, char **argv);
} // namespace utf8

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PWSTR pCmdLine, int nCmdShow) {
  int argc;
  char **argv = utf8::get_argv(&argc);
#else
int main(int argc, char **argv) {
#endif
  try {
    vivictpp::OptParser optParser;
    if (!optParser.parseOptions(argc, argv)) {
      return optParser.exitCode;
    }

    VivictPPConfig vivictPPConfig = optParser.vivictPPConfig;
#ifdef _WIN32
    utf8::free_argv(argc, argv);
#endif
    vivictPPConfig.applySettings(vivictpp::loadSettings());
    vivictpp::logging::initializeLogging(vivictPPConfig.settings);

    for (auto sourceConfig : vivictPPConfig.sourceConfigs) {
      spdlog::debug("Source: path={} filters={}", sourceConfig.path,
                    sourceConfig.filter);
    }

    vivictpp::imgui::VivictPPImGui vivictPPImGui(vivictPPConfig);
    vivictPPImGui.run();
  } catch (const std::exception &e) {
    std::cerr << "Vivict++ had an unexpected error: " << e.what() << std::endl;
    return 1;
  }
}
