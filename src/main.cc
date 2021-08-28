// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later


#include <fstream>
#include <iostream>
#include <vector>
#include "spdlog/cfg/env.h"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

#include "VivictPP.hh"
#include "SourceConfig.hh"
#include "vmaf/VmafLog.hh"

#include "CLI/App.hpp"
#include "CLI/Formatter.hpp"
#include "CLI/Config.hpp"

const std::string FOOTER =
  R"(
KEYBOARD SHORTCUTS

SPACE  Play/Pause video
,      Step forward 1 frame
.      Step backward 1 frame
/ or - Step forward 5 seconds
m      Step backward 5 seconds

f      Toggle full screen
u      Zoom in
i      Zoom out
0      Reset pan and zoom to default
t      Toggle visibility of time
d      Toggle visibility of Stream and Frame metadata
p      Toggle visibility of vmaf plot (if vmaf data present)

q      Quit application

See also  https://github.com/svt/vivictpp#readme)";

int main(int argc, char **argv) {
  try {
    CLI::App app{"Vivict++ - Vivict Video Comparison Tool ++"};
    app.footer(FOOTER);

    std::string leftVideo;
    app.add_option("leftVideo", leftVideo, "Path or url to first (left) video")->required();

    std::string rightVideo;
    app.add_option("rightVideo", rightVideo, "Path or url to second (right) video");

    std::string leftFilter("");
    std::string rightFilter("");
    app.add_option("--left-filter", leftFilter, "Video filters for left video");
    app.add_option("--right-filter", rightFilter, "Video filters for left video");

    bool disableAudio;
    app.add_flag("--disable-audio",  disableAudio, "Disable audio");

    std::string leftVmaf("");
    std::string rightVmaf("");
    app.add_option("--left-vmaf", leftVmaf, "Path to csv-file containing vmaf data for left video");
    app.add_option("--right-vmaf", rightVmaf, "Path to csv-file containing vmaf data for right video");

    CLI11_PARSE(app, argc, argv);

    std::vector<std::string> sources = {leftVideo};
    if (!rightVideo.empty()) {
      sources.push_back(rightVideo);
    }
    std::vector<std::string> filters = {leftFilter, rightFilter};
    std::vector<std::string> vmafLogfiles = {leftVmaf, rightVmaf};

    spdlog::cfg::load_env_levels();
    spdlog::set_pattern("%H:%M:%S.%e %^%=8l%$ %-20n thread-%t  %v");
    
    std::vector<SourceConfig> sourceConfigs;
    for (size_t i = 0; i<sources.size(); i++) {
        std::string filter = i < filters.size() ? filters[i] : "";
        std::string vmafLogFile = i < vmafLogfiles.size() ? vmafLogfiles[i] : "";
        sourceConfigs.push_back(SourceConfig(sources[i], filter, vmafLogFile));
    }

    for (auto sourceConfig : sourceConfigs) {
        spdlog::debug("Source: path={} filters={}", sourceConfig.path, sourceConfig.filter);
    }

    VivictPPConfig vivictPPConfig(sourceConfigs, disableAudio);
    VivictPP vivictPP(vivictPPConfig);
    return vivictPP.run();
  } catch (const std::exception &e) {
    std::cerr << "Vivict had an unexpected error: " << e.what() << std::endl;
    return 1;
  }
}
