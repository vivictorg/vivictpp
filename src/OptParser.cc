// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "OptParser.hh"

#include "Version.hh"

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

See also  https://github.com/vivictorg/vivictpp#readme

Vivict++ )" + std::string(VPP_VERSION) + " (" + std::string(VPP_GIT_HASH) + ")";


std::vector<std::string> splitString(const std::string &input) {
  std::vector<std::string> result;;
  if (input.empty()) {
    return result;
  }

  std::istringstream ss(input);
  std::string token;

  while(std::getline(ss, token, ',')) {
    result.push_back(token);
  }
  return result;
}

bool vivictpp::OptParser::parseOptions(int argc, char **argv) {
    CLI::App app{"Vivict++ - Vivict Video Comparison Tool ++"};
    app.footer(FOOTER);

    std::string leftVideo;
    app.add_option("leftVideo", leftVideo, "Path or url to first (left) video");

    std::string rightVideo;
    app.add_option("rightVideo", rightVideo, "Path or url to second (right) video");

    std::string leftFilter("");
    std::string rightFilter("");
    app.add_option("--left-filter", leftFilter, "Video filters for left video");
    app.add_option("--right-filter", rightFilter, "Video filters for left video");

    bool enableAudio(false);
    app.add_flag("--enable-audio",  enableAudio, "Enable audio");

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

    std::string preferredDecodersStr;
    app.add_option("--preferred-decoders", preferredDecodersStr,
                   std::string("Comma separated list of decoders that should be preferred over default decoder when applicable"));

    //CLI11_PARSE(app, argc, argv);
    try {
      app.parse(argc, argv);
    } catch (const CLI::ParseError &e) {
      this->exitCode =  app.exit(e);
      return false;
    }

    std::vector<std::string> sources;
    if (!leftVideo.empty()) {
      sources.push_back(leftVideo);
    }
    if (!rightVideo.empty()) {
      sources.push_back(rightVideo);
    }
    std::vector<std::string> filters = {leftFilter, rightFilter};
//    std::vector<std::string> vmafLogfiles = {leftVmaf, rightVmaf};
    std::vector<std::string> formatOptions = {leftInputFormat, rightInputFormat};
    std::vector<std::string> preferredDecoders = splitString(preferredDecodersStr);
    std::vector<std::string> hwAccels = splitString(hwAccel);

    std::vector<SourceConfig> sourceConfigs;
    for (size_t i = 0; i<sources.size(); i++) {
        std::string filter = i < filters.size() ? filters[i] : "";
//        std::string vmafLogFile = i < vmafLogfiles.size() ? vmafLogfiles[i] : "";
        std::string format = i < formatOptions.size() ? formatOptions[i] : "";
        sourceConfigs.push_back(SourceConfig(sources[i], hwAccels,
                                             splitString(preferredDecodersStr),filter, format));
    }

    this->vivictPPConfig =
      VivictPPConfig(sourceConfigs, !enableAudio, {disableFontAutoScaling, fontCustomScaling},  {hwAccels, preferredDecoders});
    return true;
  };
