// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef LOGGING_LOGGING_HH
#define LOGGING_LOGGING_HH

#include <vector>
#include <string>
#include <map>

#include "spdlog/spdlog.h"
#include "time/Time.hh"

#include "Settings.hh"


namespace vivictpp {
namespace logging {

using Logger = std::shared_ptr<spdlog::logger>;

const std::string LIBAV_LOGGER_NAME = "libav";
const std::string DEFAULT_LOGGER_NAME = "default";
const std::string DEFAULT_LEVEL = "(default)";


Logger getOrCreateLogger(std::string name);

void initializeLogging(const vivictpp::Settings &settings);

const std::vector<std::string> getMessages();

const std::vector<std::string> &getLoggers();

const std::map<std::string, spdlog::level::level_enum> &getLogLevels();

void setLogLevels(const std::map<std::string, std::string> &logLevels, bool createLoggers = false);

}  // namespace logging
}  // namespace vivictpp

#endif // LOGGING_LOGGING_HH
