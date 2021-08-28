// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "logging/Logging.hh"
#include "spdlog/sinks/stdout_color_sinks.h"

vivictpp::logging::Logger vivictpp::logging::getOrCreateLogger(std::string name) {
    Logger logger = spdlog::get(name);
    if (!logger) {
        logger = spdlog::stdout_color_mt(name);
    }
    return logger;
}
