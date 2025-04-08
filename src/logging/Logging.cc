// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later
#include "logging/Logging.hh"
#include "spdlog/common.h"

#include <cstdlib>
#include <exception>
#include <map>
#include <memory>
#include <mutex>
#include <string>

extern "C" {
#include <libavformat/avformat.h>
}

#include "spdlog/cfg/env.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/ringbuffer_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

/*
LOGGERS
"vivictpp::workers::DecoderWorker"
"vivictpp::workers::PacketWorker"
"vivictpp::libav::FormatHandler"
"vivictpp::seeklog"
"vivictpp::libav::Decoder"
"default"
"SeekState"
"VideoInputs"
"vivictpp::VideoPlayback"
"vivictpp::workers::FrameBuffer"
 */

static std::vector<spdlog::sink_ptr> sinks;
static std::shared_ptr<spdlog::sinks::ringbuffer_sink_mt> ringbufferSink;
static const std::vector<std::string> loggers{
    "default",
    "vivictpp::workers::DecoderWorker",
    "vivictpp::workers::PacketWorker",
    "vivictpp::libav::FormatHandler",
    "vivictpp::seeklog",
    "vivictpp::libav::Decoder",
    "SeekState",
    "VideoInputs",
    "vivictpp::VideoPlayback",
    "vivictpp::workers::FrameBuffer",
    "vivictpp::video::VideoIndexer",
    "libav",
    "vivictpp::qualityMetrics::QualityMetrics"};

// static std::mutex loggersMutex;
static const std::map<std::string, spdlog::level::level_enum> logLevels{
    {"trace", spdlog::level::trace}, {"debug", spdlog::level::debug},
    {"info", spdlog::level::info},   {"warning", spdlog::level::warn},
    {"error", spdlog::level::err},   {"critical", spdlog::level::critical},
    {"off", spdlog::level::off}};

static const std::map<int, spdlog::level::level_enum> avLogLevelToSpdLogLevel =
    {
        {AV_LOG_QUIET, spdlog::level::off},
        {AV_LOG_PANIC, spdlog::level::critical},
        {AV_LOG_FATAL, spdlog::level::critical},
        {AV_LOG_ERROR, spdlog::level::err},
        {AV_LOG_WARNING, spdlog::level::warn},
        {AV_LOG_INFO, spdlog::level::info},
        {AV_LOG_VERBOSE, spdlog::level::debug},
        {AV_LOG_DEBUG, spdlog::level::debug},
        {AV_LOG_TRACE, spdlog::level::trace},
};

static const std::map<spdlog::level::level_enum, int> spdLogLevelToAvLogLevel =
    {
        {spdlog::level::off, AV_LOG_QUIET},
        {spdlog::level::critical, AV_LOG_FATAL},
        {spdlog::level::err, AV_LOG_ERROR},
        {spdlog::level::warn, AV_LOG_WARNING},
        {spdlog::level::info, AV_LOG_INFO},
        {spdlog::level::debug, AV_LOG_VERBOSE},
        {spdlog::level::debug, AV_LOG_DEBUG},
        {spdlog::level::trace, AV_LOG_TRACE},
};

static int avLogLevel = AV_LOG_WARNING;
const int avLogLineSize = 1024;
static std::mutex avLogMutex;

void avLogCallback(void *avcl, int level, const char *fmt, va_list vl) {
  char line[avLogLineSize];
  static int printPrefix = 1;
  static vivictpp::logging::Logger logger =
      vivictpp::logging::getOrCreateLogger("libav");
  std::lock_guard<std::mutex> lock(avLogMutex);
  if (level > avLogLevel) {
    return;
  }
  int n = av_log_format_line2(avcl, level, fmt, vl, line, avLogLineSize,
                              &printPrefix);
  if (n > 1) {
    // Remove newline
    line[std::min(avLogLineSize, n) - 2] = '\0';
  }
  if (!avLogLevelToSpdLogLevel.count(level)) {
    // TODO: log warning
    return;
  }
  const spdlog::level::level_enum spdLogLevel =
      avLogLevelToSpdLogLevel.at(level);
  logger->log(spdLogLevel, "{}", line);
}

const std::vector<std::string> vivictpp::logging::getMessages() {
  return ringbufferSink->last_formatted();
}

vivictpp::logging::Logger
vivictpp::logging::getOrCreateLogger(std::string name) {
  spdlog::debug("getOrCreateLogger {}", name);
  Logger logger = spdlog::get(name);
  if (!logger) {
    spdlog::debug("Creating logger {}", name);
    logger = std::make_shared<spdlog::logger>(name, std::begin(sinks),
                                              std::end(sinks));
    spdlog::initialize_logger(logger);
  }
  return logger;
}

const std::vector<std::string> &vivictpp::logging::getLoggers() {
  //  std::lock_guard<std::mutex> lock(loggersMutex);
  return loggers;
}

const std::map<std::string, spdlog::level::level_enum> &
vivictpp::logging::getLogLevels() {
  return logLevels;
}

int avLogLevelFromName(const char *logLevelName) {
  static std::map<std::string, int> logLevelNames = {
      {"quiet", AV_LOG_QUIET},     {"panic", AV_LOG_PANIC},
      {"fatal", AV_LOG_FATAL},     {"error", AV_LOG_ERROR},
      {"warning", AV_LOG_WARNING}, {"info", AV_LOG_INFO},
      {"verbose", AV_LOG_VERBOSE}, {"debug", AV_LOG_DEBUG},
      {"trace", AV_LOG_TRACE},
  };

  std::string key(logLevelName);

  if (logLevelNames.find(key) == logLevelNames.end()) {
    throw std::runtime_error(
        std::string("Unknown value for env variable AVLOG_LEVEL: ") + key);
  }

  return logLevelNames[key];
}

void vivictpp::logging::setLogLevels(
    const std::map<std::string, std::string> &newLevels, bool createLoggers) {
  spdlog::level::level_enum defaultLevel =
      logLevels.at(newLevels.at(DEFAULT_LOGGER_NAME));
  spdlog::set_level(defaultLevel);
  for (auto l : newLevels) {
    auto loggerName = l.first;
    auto levelName = l.second;
    if (loggerName == DEFAULT_LOGGER_NAME) {
      continue;
    }
    if (loggerName == LIBAV_LOGGER_NAME) {
      std::lock_guard<std::mutex> lock(avLogMutex);
      spdlog::level::level_enum level =
          levelName == "(default)" ? defaultLevel : logLevels.at(levelName);
      avLogLevel = spdLogLevelToAvLogLevel.at(level);
    }
    Logger logger =
        createLoggers ? getOrCreateLogger(loggerName) : spdlog::get(loggerName);
    if (!logger) {
      continue;
    }
    spdlog::level::level_enum level =
        levelName == "(default)" ? defaultLevel : logLevels.at(levelName);
    logger->set_level(level);
  }
}

void initSinks(const vivictpp::Settings &settings) {
  ringbufferSink = std::make_shared<spdlog::sinks::ringbuffer_sink_mt>(
      settings.logBufferSize);
  sinks.push_back(ringbufferSink);
  if (settings.logToFile) {
    auto fileSink =
        std::make_shared<spdlog::sinks::basic_file_sink_mt>(settings.logFile);
    sinks.push_back(fileSink);
  }
}

void vivictpp::logging::initializeLogging(const vivictpp::Settings &settings) {
  spdlog::set_level(spdlog::level::info);
  initSinks(settings);
  spdlog::set_default_logger(getOrCreateLogger("default"));
  spdlog::set_pattern("%H:%M:%S.%e %^%=8l%$ %-20n thread-%t  %v");
  setLogLevels(settings.logLevels, true);
  av_log_set_callback(&avLogCallback);
  // Ensure all loggers initialized to avoid race conditions
  for (auto logger : loggers) {
    getOrCreateLogger(logger);
  }
}
