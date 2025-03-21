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

vivictpp::qualitymetrics::QualityMetrics::QualityMetrics(
    std::string metricsFile) {
  if (endsWith(metricsFile, ".csv")) {
    metrics = parseCsvFile(metricsFile, {"vmaf_hd", "vmaf"});
  } else if (endsWith(metricsFile, ".json")) {
    metrics = parseJsonFile(metricsFile, {"vmaf_hd", "vmaf"});
  } else {
    throw std::invalid_argument("Invalid metrics file format");
  }
}

vivictpp::qualitymetrics::QualityMetrics
vivictpp::qualitymetrics::QualityMetrics::loadMetricsForSource(
    std::string sourceFile) {
  spdlog::warn("Loading metrics for source: {}", sourceFile);
  std::string jsonPath = std::filesystem::path(sourceFile)
                             .replace_extension()
                             .string() + "_vmaf.json";
  spdlog::warn("Checking for json file: {}", jsonPath);
  if (std::filesystem::exists(jsonPath)) {
    return QualityMetrics(jsonPath);
  }
  std::string csvPath =
      std::filesystem::path(sourceFile).replace_extension().string() + "_vmaf.csv";
  spdlog::warn("Checking for csv file: {}", csvPath);
  if (std::filesystem::exists(csvPath)) {
    return QualityMetrics(csvPath);
  }
  spdlog::warn("No metrics found for source: {}", sourceFile);
  return QualityMetrics();
}

std::map<std::string, std::vector<float>>
parseJsonFile(std::string metricsFile, std::vector<std::string> metricNames) {
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
      spdlog::warn("Metric: {} Value: {}", metric, value);
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
  for (auto &metric : metrics) {
    result[metric] = std::vector<float>();
  }
  std::ifstream infile(logfile);
  std::string line;
  std::getline(infile, line);
  std::map<std::string, int> headers;
  getCsvHeaders(line, headers);
  while (std::getline(infile, line)) {
    auto values = parseCsvLine(line);
    for (auto &metric : metrics) {
      result[metric].push_back(values[headers[metric]]);
    }
  }
  return result;
}
