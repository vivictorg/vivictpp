// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch2/catch.hpp"

#include "Settings.hh"

void requireSettingsEquals(const vivictpp::Settings &lhs, const vivictpp::Settings &rhs);

TEST_CASE( "Empty settings file" "[Settings]") {
    vivictpp::Settings settings = vivictpp::loadSettings("../testdata/settings/empty_settings.toml");
    vivictpp::Settings expectedSettings;
    requireSettingsEquals(settings, expectedSettings);
}

TEST_CASE( "Incomplete settings file" "[Settings]") {
    vivictpp::Settings settings = vivictpp::loadSettings("../testdata/settings/incomplete_settings.toml");
    vivictpp::Settings expectedSettings;
    expectedSettings.disableFontAutoScaling;
    expectedSettings.baseFontSize = 14;
    expectedSettings.hwAccels = {"vaapi"};
    expectedSettings.preferredDecoders = {};
    expectedSettings.logBufferSize = 128;
    expectedSettings.logToFile = false;
    expectedSettings.logFile = "";
    expectedSettings.logLevels = {
            {"SeekState", "warn"},
            {"RandomLog", "error"}
    };
    requireSettingsEquals(settings, expectedSettings);
}

TEST_CASE( "Complete settings file" "[Settings]") {
    vivictpp::Settings expectedSettings;
    expectedSettings.disableFontAutoScaling = true;
    expectedSettings.baseFontSize = 18;
    expectedSettings.hwAccels = {"vaapi"};
    expectedSettings.preferredDecoders = {"libopenjpeg"};
    expectedSettings.logBufferSize = 256;
    expectedSettings.logToFile = true;
    expectedSettings.logFile = "/tmp/vivictpp.log";
    expectedSettings.logLevels = {
            {"SeekState", "warn"},
            {"RandomLog", "error"}
    };
    vivictpp::Settings settings = vivictpp::loadSettings("../testdata/settings/complete_settings.toml");
    requireSettingsEquals(settings, expectedSettings);
}

void requireSettingsEquals(const vivictpp::Settings &lhs, const vivictpp::Settings &rhs) {
    REQUIRE(lhs.baseFontSize == rhs.baseFontSize);
    REQUIRE(lhs.disableFontAutoScaling == rhs.disableFontAutoScaling);
    REQUIRE(lhs.hwAccels == rhs.hwAccels);
    REQUIRE(lhs.preferredDecoders == rhs.preferredDecoders);
    REQUIRE(lhs.logBufferSize == rhs.logBufferSize);
    REQUIRE(lhs.logToFile == rhs.logToFile);
    REQUIRE(lhs.logFile == rhs.logFile);
    REQUIRE(lhs.logLevels == rhs.logLevels);
}
