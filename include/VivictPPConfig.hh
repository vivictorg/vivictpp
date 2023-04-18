// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIVICTPPCONFIG_HH
#define VIVICTPPCONFIG_HH

#include <string>
#include <vector>
#include <algorithm>
#include "SourceConfig.hh"
#include "vmaf/VmafLog.hh"
#include "libav/DecoderOptions.hh"

struct UiOptions {
  bool disableFontAutoScaling;
  float fontCustomScaling;
  bool enableImGui;
};

class VivictPPConfig {
public:
  VivictPPConfig(): VivictPPConfig({}, false, {false, 1.0, false}, {}) {}

  VivictPPConfig(std::vector<SourceConfig> sourceConfigs,
                 bool disableAudio,
                 UiOptions uiOptions,
                 vivictpp::libav::DecoderOptions decoderOptions):
    sourceConfigs(sourceConfigs),
    disableAudio(disableAudio),
    uiOptions(uiOptions),
    decoderOptions(decoderOptions) {}

  std::vector<SourceConfig> sourceConfigs;

  bool disableAudio;

  UiOptions uiOptions;

  vivictpp::libav::DecoderOptions decoderOptions;

public:
  bool hasVmafData() {
    return std::any_of(sourceConfigs.begin(),
                sourceConfigs.end(),
                [](const SourceConfig &sc) { return !sc.vmafLog.empty(); });
  }
};

#endif // VIVICTPPCONFIG_HH
