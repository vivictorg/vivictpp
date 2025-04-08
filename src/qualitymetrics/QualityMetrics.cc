// SPDX-FileCopyrightText: 2025 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "qualitymetrics/QualityMetrics.hh"

#include "json.hpp"
#include "spdlog/spdlog.h"
#include <fstream>
#include <map>
#include <ostream>
#include <sstream>
inline bool endsWith(std::string const &value, std::string const &ending) {
  if (ending.size() > value.size())
    return false;
  return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

std::map<std::string, std::vector<float>>
parseCsvFile(std::string logfile, std::vector<std::string> metrics);

std::map<std::string, std::vector<float>>
parseJsonFile(std::string metricsFile, std::vector<std::string> metricNames);

void vivictpp::qualitymetrics::QualityMetricsLoader::loadMetrics(
    std::string metricsFile,
    vivictpp::qualitymetrics::QualityMetricsLoaderCallback callback) {
  stopLoaderThread();
  stopLoader = false;
  loaderThread = std::make_unique<std::thread>(
      &QualityMetricsLoader::loadMetricsInternal, this, metricsFile, callback);
}

void vivictpp::qualitymetrics::QualityMetricsLoader::autoloadMetrics(
    std::string sourceFile,
    vivictpp::qualitymetrics::QualityMetricsLoaderCallback callback) {
       auto logger = vivictpp::logging::getOrCreateLogger("vivictpp::qualityMetrics::QualityMetrics");
  logger->info("Autoloading metrics for source: {}", sourceFile);
  std::string jsonPath =
      std::filesystem::path(sourceFile).replace_extension().string() +
      "_vmaf.json";
  logger->info("Checking for json file: {}", jsonPath);
  if (std::filesystem::exists(jsonPath)) {
    return loadMetrics(jsonPath, callback);
  }
  std::string csvPath =
      std::filesystem::path(sourceFile).replace_extension().string() +
      "_vmaf.csv";
  logger->info("Checking for csv file: {}", csvPath);
  if (std::filesystem::exists(csvPath)) {
    return loadMetrics(csvPath, callback);
  }
  logger->info("No metrics found for source: {}", sourceFile);
}

void vivictpp::qualitymetrics::QualityMetricsLoader::stopLoaderThread() {
  if (loaderThread) {
    stopLoader = true;
    loaderThread->join();
    loaderThread.reset();
  }
}

void vivictpp::qualitymetrics::QualityMetricsLoader::loadMetricsInternal(
    std::string sourceFile,
    vivictpp::qualitymetrics::QualityMetricsLoaderCallback callback) {
  std::shared_ptr<QualityMetrics> metrics;
  try {
    metrics = std::make_shared<QualityMetrics>(sourceFile);
  } catch (const std::exception &e) {
    logger->warn("Error loading metrics for source: {}", sourceFile);
    callback(nullptr, std::make_unique<std::exception>(e));
  }
  callback(metrics, nullptr);
}

vivictpp::qualitymetrics::QualityMetrics::QualityMetrics(
    std::string metricsFile) {
      std::vector<std::string> metricsToLoad{"vmaf", "vmaf_hd", "integer_motion", "integer_motion2"};
  if (endsWith(metricsFile, ".csv")) {
    metrics = parseCsvFile(metricsFile, metricsToLoad);
  } else if (endsWith(metricsFile, ".json")) {
    metrics = parseJsonFile(metricsFile, metricsToLoad);
  } else {
    throw std::invalid_argument("Invalid metrics file format");
  }
}

std::map<std::string, std::vector<float>>
parseJsonFile(std::string metricsFile, std::vector<std::string> metricNames) {
  auto logger = vivictpp::logging::getOrCreateLogger("vivictpp::qualityMetrics::QualityMetrics");
  std::map<std::string, std::vector<float>> metrics;
  std::ifstream infile(metricsFile);
  nlohmann::json json;
  nlohmann::json data = nlohmann::json::parse(infile);

  for (auto &frame : data["frames"]) {
    for (auto &metric : metricNames) {
      if (!frame["metrics"].contains(metric)) {
        continue;
      }
      if (!metrics.count(metric)) {
        metrics[metric] = std::vector<float>();
      }
      float value = frame["metrics"][metric].get<float>();
      logger->warn("Metric: {} Value: {}", metric, value);
      metrics[metric].push_back(value);
    }
  }
  return metrics;
}

void getCsvHeaders(std::string line, std::map<std::string, int> &headers) {
  std::stringstream ss(line);
  std::string header;
  int i = 0;
  while (ss.good()) {
    getline(ss, header, ',');
    headers[header] = i++;
    if (ss.peek() == ',')
      ss.ignore();
  }
}

std::vector<float> parseCsvLine(std::string line) {
  std::vector<float> result;
  std::stringstream ss(line);
  float value;
  for (int i = 0; ss >> value; i++) {
    result.push_back(value);
    if (ss.peek() == ',')
      ss.ignore();
  }
  return result;
}

std::map<std::string, std::vector<float>>
parseCsvFile(std::string logfile, std::vector<std::string> metrics) {
  std::map<std::string, std::vector<float>> result;
  if (logfile.empty()) {
    return result;
  }
  std::ifstream infile(logfile);
  std::string line;
  std::getline(infile, line);
  std::map<std::string, int> headers;
  getCsvHeaders(line, headers);
  
  std::vector<std::string> existingMetrics;
  for (auto &metric: metrics) {
    if (headers.find(metric) != headers.end()) {
      existingMetrics.push_back(metric);
      result[metric] = std::vector<float>();
    }
  }
  while (std::getline(infile, line)) {
    auto values = parseCsvLine(line);
    for (auto &metric : existingMetrics) {
      result[metric].push_back(values[headers[metric]]);
    }
  }
  return result;
}
