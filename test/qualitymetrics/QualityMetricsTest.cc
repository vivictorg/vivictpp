// SPDX-FileCopyrightText: 2025 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#define CATCH_CONFIG_MAIN 
#include "catch2/catch.hpp"
#include "qualitymetrics/QualityMetrics.hh"

bool closeEnough(double a, double b) { return abs(a - b) < 0.00001; }

TEST_CASE("QualityMetricsTest", "[QualityMetrics]") {
    vivictpp::qualitymetrics::QualityMetrics metrics("../testdata/vmaf.json");
    REQUIRE(metrics.getMetrics().size() == 1);
    auto vmafHd = metrics.getMetric("vmaf_hd");
    REQUIRE(vmafHd.size() == 250);
    REQUIRE(closeEnough(vmafHd[0], 93.869087));
    REQUIRE(closeEnough(vmafHd[249], 95.772802));
}

