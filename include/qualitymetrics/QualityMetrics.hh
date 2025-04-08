// SPDX-FileCopyrightText: 2025 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef QUALITYMETRICS_QUALITYMETRICS_HH
#define QUALITYMETRICS_QUALITYMETRICS_HH

#include <map>
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include "logging/Logging.hh"

namespace vivictpp {
namespace qualitymetrics {

struct PooledMetrics {
  float min;
  float max;
  float mean;
  float harmonicMean;
};

class QualityMetrics;

typedef std::function<void(std::shared_ptr<QualityMetrics>, std::unique_ptr<std::exception>)>
    QualityMetricsLoaderCallback;

class QualityMetricsLoader {
public:
  QualityMetricsLoader()
      : logger(vivictpp::logging::getOrCreateLogger(
            "vivictpp::qualitymetrics::QualityMetricsLoader")) {}
  ~QualityMetricsLoader() { stopLoaderThread(); }
  void loadMetrics(std::string sourceFile,
                   QualityMetricsLoaderCallback callback);

private:
  void loadMetricsInternal(std::string sourceFile,
                           QualityMetricsLoaderCallback callback);
  void stopLoaderThread();

private:
  QualityMetricsLoaderCallback callback;
  vivictpp::logging::Logger logger;
  std::unique_ptr<std::thread> loaderThread;
  std::atomic_bool stopLoader{false};
};

class QualityMetrics {
public:
  static QualityMetrics loadMetricsForSource(std::string sourceFile);
  QualityMetrics() = default;
  QualityMetrics(std::string metricsFile);
  QualityMetrics(const QualityMetrics& other) = default;
  QualityMetrics& operator=(const QualityMetrics& other) = default;
  ~QualityMetrics() = default;

  std::vector<std::string> getMetrics() const {
    std::vector<std::string> keys;
    for (const auto &pair : metrics) {
      keys.push_back(pair.first);
    }
    return keys;
  }

  const std::vector<float> &getMetric(const std::string &metric) const {
    return metrics.at(metric);
  }

  const PooledMetrics &getPooledMetric(const std::string &metric) const {
    return pooledMetrics.at(metric);
  }

  const bool hasMetric(const std::string &metric) const {
    return metrics.find(metric) != metrics.end();
  }

  bool empty() const { return metrics.empty(); }

private:
  std::map<std::string, std::vector<float>> metrics;
  std::map<std::string, PooledMetrics> pooledMetrics;
};

} // namespace qualitymetrics
} // namespace vivictpp

#endif // QUALITYMETRICS_QUALITYMETRICS_HH