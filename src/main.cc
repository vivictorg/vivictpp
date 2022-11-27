// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later


#include <fstream>
#include <iostream>
#include <vector>

#include "SDL_video.h"
#include "sdl/SDLUtils.hh"
#include "spdlog/spdlog.h"
#include "ui/FontSize.hh"

#include "Version.hh"
#include "VivictPP.hh"
#include "Controller.hh"
#include "SourceConfig.hh"
#include "vmaf/VmafLog.hh"

#include "CLI/App.hpp"
#include "CLI/Formatter.hpp"
#include "CLI/Config.hpp"

#ifndef VPP_VERSION
#define VPP_VERSION "unknown version"
#endif

const std::string FOOTER =
  R"(
KEYBOARD SHORTCUTS

SPACE  Play/Pause video
,      Step forward 1 frame
.      Step backward 1 frame
/ or - Seek forward 5 seconds
m      Seek backward 5 seconds
?      Seek Forward 60s
M      Seek backward 60s
Alt-?  Seek Forward 10min
Alt-M  Seek Backward 10min
<      Decrease left frame offset
>      Increase left frame offset
[      Decrease playback speed
]      Increase playback speed

f      Toggle full screen
u      Zoom in
i      Zoom out
0      Reset pan and zoom to default
s      Toggle scale content to fit window
t      Toggle visibility of time
d      Toggle visibility of Stream and Frame metadata
p      Toggle visibility of vmaf plot (if vmaf data present)

q      Quit application

See also  https://github.com/svt/vivictpp#readme

Vivict++ )" + std::string(VPP_VERSION) + " " + std::string(VPP_GIT_HASH);

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

    bool enableAudio(false);
    app.add_flag("--enable-audio",  enableAudio, "Enable audio");

    std::string leftVmaf("");
    std::string rightVmaf("");
    app.add_option("--left-vmaf", leftVmaf, "Path to csv-file containing vmaf data for left video");
    app.add_option("--right-vmaf", rightVmaf, "Path to csv-file containing vmaf data for right video");

    std::string leftInputFormat;
    std::string rightInputFormat;
    app.add_option("--left-format", leftInputFormat, "Format options for left video input");
    app.add_option("--right-format", rightInputFormat, "Format options for right video input");

    bool disableFontAutoScaling(false);
    float fontCustomScaling{1};
    app.add_flag("--disable-font-autoscaling", disableFontAutoScaling, "Disables autoscaling of fonts based on display dpi");
    app.add_option("--custom-font-scaling", fontCustomScaling, "Custom scaling factor for fonts");

    std::string hwAccel("none");
    app.add_option("--hwaccel", hwAccel,
                   std::string("Select device type to use for hardware accelerated decoding. Valid values are:\n") +
                   "  auto    Use any available device type (default)\n" +
                   "  none    Disable hardware accelerated decoding\n" +
                   "  TYPE    Name of devicetype, see https://trac.ffmpeg.org/wiki/HWAccelIntro");

    CLI11_PARSE(app, argc, argv);

    std::vector<std::string> sources = {leftVideo};
    if (!rightVideo.empty()) {
      sources.push_back(rightVideo);
    }
    std::vector<std::string> filters = {leftFilter, rightFilter};
    std::vector<std::string> vmafLogfiles = {leftVmaf, rightVmaf};
    std::vector<std::string> formatOptions = {leftInputFormat, rightInputFormat};

    vivictpp::logging::initializeLogging();

    std::vector<SourceConfig> sourceConfigs;
    for (size_t i = 0; i<sources.size(); i++) {
        std::string filter = i < filters.size() ? filters[i] : "";
        std::string vmafLogFile = i < vmafLogfiles.size() ? vmafLogfiles[i] : "";
        std::string format = i < formatOptions.size() ? formatOptions[i] : "";
        sourceConfigs.push_back(SourceConfig(sources[i], filter, vmafLogFile, format, {hwAccel}));
    }

    for (auto sourceConfig : sourceConfigs) {
        spdlog::debug("Source: path={} filters={}", sourceConfig.path, sourceConfig.filter);
    }

    VivictPPConfig vivictPPConfig(sourceConfigs, !enableAudio);
    vivictpp::sdl::SDLInitializer sdlInitializer(enableAudio);
    vivictpp::ui::FontSize::setScaling(!disableFontAutoScaling, fontCustomScaling);
    auto sdlEventLoop = std::make_shared<vivictpp::sdl::SDLEventLoop>(vivictPPConfig.sourceConfigs);
    vivictpp::Controller controller(sdlEventLoop, sdlEventLoop, vivictPPConfig);
    return controller.run();
  } catch (const std::exception &e) {
    std::cerr << "Vivict had an unexpected error: " << e.what() << std::endl;
    return 1;
  }
}
