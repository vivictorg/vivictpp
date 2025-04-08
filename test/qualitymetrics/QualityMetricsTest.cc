// SPDX-FileCopyrightText: 2025 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#define CATCH_CONFIG_MAIN 
#include "catch2/catch.hpp"
#include "qualitymetrics/QualityMetrics.hh"

bool closeEnough(double a, double b) { return abs(a - b) < 0.0001; }

TEST_CASE("Load quality metrics JSON", "[QualityMetrics]") {
    vivictpp::qualitymetrics::QualityMetrics metrics("../testdata/vmaf.json");
    REQUIRE(metrics.getMetrics().size() == 1);
    auto vmafHd = metrics.getMetric("vmaf_hd");
    REQUIRE(vmafHd.size() == 250);
    REQUIRE(abs(vmafHd[0] - 93.879087) < 0.0001);
    REQUIRE(abs(vmafHd[249] - 95.772802) < 0.0001);
}

TEST_CASE("Load quality metrics CSV", "[QualityMetrics]") {
    vivictpp::qualitymetrics::QualityMetrics metrics("../testdata/vmaf.csv");
    REQUIRE(metrics.getMetrics().size() == 1);
    auto vmafHd = metrics.getMetric("vmaf");
    REQUIRE(vmafHd.size() == 250);
    REQUIRE(abs(vmafHd[0] - 95.009138) < 0.0001);
    REQUIRE(abs(vmafHd[249] - 97.881930) < 0.0001);
}

