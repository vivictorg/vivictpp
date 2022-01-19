// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "logging/Logging.hh"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/cfg/env.h"

vivictpp::logging::Logger vivictpp::logging::getOrCreateLogger(std::string name) {
    Logger logger = spdlog::get(name);
    if (!logger) {
        logger = spdlog::stdout_color_mt(name);
    }
    return logger;
}


void vivictpp::logging::initializeLogging() {
  spdlog::cfg::load_env_levels();
  spdlog::set_pattern("%H:%M:%S.%e %^%=8l%$ %-20n thread-%t  %v");
}
