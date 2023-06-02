// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#define SDL_MAIN_HANDLED

#include <fstream>
#include <iostream>
#include <vector>
#include <string>

#include "SDL_video.h"
#include "sdl/SDLUtils.hh"
#include "spdlog/spdlog.h"
#include "ui/FontSize.hh"

#include "OptParser.hh"
#include "VivictPP.hh"
#include "Controller.hh"
#include "SourceConfig.hh"
#include "vmaf/VmafLog.hh"
#include "imgui/VivictPPImGui.hh"
#include "Settings.hh"

int main(int argc, char **argv) {
  try {
    vivictpp::OptParser optParser;
    if (!optParser.parseOptions(argc, argv)) {
      return optParser.exitCode;
    }
    VivictPPConfig vivictPPConfig = optParser.vivictPPConfig;
    vivictPPConfig.applySettings(vivictpp::loadSettings());
    vivictpp::logging::initializeLogging();


    for (auto sourceConfig : vivictPPConfig.sourceConfigs) {
        spdlog::debug("Source: path={} filters={}", sourceConfig.path, sourceConfig.filter);
    }
    if (vivictPPConfig.uiOptions.enableImGui) {
      vivictpp::imgui::VivictPPImGui vivictPPImGui(vivictPPConfig);
      vivictPPImGui.run();
    } else {

      vivictpp::sdl::SDLInitializer sdlInitializer(!vivictPPConfig.disableAudio);
      vivictpp::ui::FontSize::setScaling(!vivictPPConfig.uiOptions.disableFontAutoScaling,
                                         vivictPPConfig.uiOptions.fontCustomScaling);
      auto sdlEventLoop = std::make_shared<vivictpp::sdl::SDLEventLoop>(vivictPPConfig.sourceConfigs);
      vivictpp::Controller controller(sdlEventLoop, sdlEventLoop, vivictPPConfig);
      return controller.run();
    }
  } catch (const std::exception &e) {
    std::cerr << "Vivict had an unexpected error: " << e.what() << std::endl;
    return 1;
  }
}
