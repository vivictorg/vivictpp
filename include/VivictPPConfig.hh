// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIVICTPPCONFIG_HH
#define VIVICTPPCONFIG_HH

#include "Settings.hh"
#include "SourceConfig.hh"
#include "libav/DecoderOptions.hh"
#include "vmaf/VmafLog.hh"
#include <algorithm>
#include <string>
#include <vector>

class VivictPPConfig {
public:
  VivictPPConfig() : VivictPPConfig({}, false, {}, false) {}

  VivictPPConfig(std::vector<SourceConfig> sourceConfigs, bool disableAudio,
                 vivictpp::libav::DecoderOptions decoderOptions,
    bool blindTest)
      : sourceConfigs(sourceConfigs), disableAudio(disableAudio),
        blindTest(blindTest),
        decoderOptions(decoderOptions) {}

  std::vector<SourceConfig> sourceConfigs;

  bool disableAudio;

  bool blindTest;

  vivictpp::libav::DecoderOptions decoderOptions;

  vivictpp::Settings settings;

  void applySettings(const vivictpp::Settings &settings) {
    this->settings = settings;
    for (auto &sourceConfig : sourceConfigs) {
      if (sourceConfig.hwAccels.empty() || sourceConfig.hwAccels[0].empty()) {
        sourceConfig.hwAccels = settings.hwAccels;
      } else if (sourceConfig.hwAccels[0] == "none") {
        sourceConfig.hwAccels = {};
      }
      if (sourceConfig.preferredDecoders.empty() ||
          sourceConfig.preferredDecoders[0].empty()) {
        sourceConfig.preferredDecoders = settings.preferredDecoders;
      }
    }
  }
};

#endif // VIVICTPPCONFIG_HH
