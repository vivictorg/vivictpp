// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later
#include "logging/Logging.hh"

#include <cstdlib>
#include <map>
#include <string>
#include <exception>

extern "C" {
#include <libavformat/avformat.h>
}


#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/cfg/env.h"



vivictpp::logging::Logger vivictpp::logging::getOrCreateLogger(std::string name) {
    Logger logger = spdlog::get(name);
    if (!logger) {
        logger = spdlog::stdout_color_mt(name);
    }
    return logger;
}

int avLogLevelFromName(const char* logLevelName) {
  static std::map<std::string, int> logLevelNames = {
    { "quiet"  , AV_LOG_QUIET   },
    { "panic"  , AV_LOG_PANIC   },
    { "fatal"  , AV_LOG_FATAL   },
    { "error"  , AV_LOG_ERROR   },
    { "warning", AV_LOG_WARNING },
    { "info"   , AV_LOG_INFO    },
    { "verbose", AV_LOG_VERBOSE },
    { "debug"  , AV_LOG_DEBUG   },
    { "trace"  , AV_LOG_TRACE   },
  };

  std::string key(logLevelName);

  if(logLevelNames.find(key) == logLevelNames.end()) {
    throw std::runtime_error(std::string("Unknown value for env variable AVLOG_LEVEL: ") + key);
  }

  return logLevelNames[key];
}



void setAvLogLevel() {
    char* avlogLevelStr = std::getenv("AVLOG_LEVEL");
    int avLogLevel = avlogLevelStr ? avLogLevelFromName(avlogLevelStr) : AV_LOG_WARNING;
    av_log_set_level(avLogLevel);
    spdlog::info("av log level: {}", avLogLevel);
}

void vivictpp::logging::initializeLogging() {
  if (std::getenv("SPDLOG_LEVEL")) {
      spdlog::cfg::load_env_levels();
  } else {
      spdlog::set_level(spdlog::level::warn);
  }
  spdlog::set_pattern("%H:%M:%S.%e %^%=8l%$ %-20n thread-%t  %v");
  setAvLogLevel();
}
