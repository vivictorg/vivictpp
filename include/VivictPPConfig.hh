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

class VivictPPConfig {
public:
  VivictPPConfig(): VivictPPConfig({}, false, false, 1.0) {}
  
  VivictPPConfig(std::vector<SourceConfig> sourceConfigs,
                 bool disableAudio,
                 bool disableFontAutoScaling,
                 float fontCustomScaling):
    sourceConfigs(sourceConfigs),
    disableAudio(disableAudio),
    disableFontAutoScaling(disableFontAutoScaling),
    fontCustomScaling(fontCustomScaling) {}

  std::vector<SourceConfig> sourceConfigs;

  bool disableAudio;

  bool disableFontAutoScaling;

  float fontCustomScaling;

public:
  bool hasVmafData() {
    return std::any_of(sourceConfigs.begin(),
                sourceConfigs.end(),
                [](const SourceConfig &sc) { return !sc.vmafLog.empty(); });
  }
};

#endif // VIVICTPPCONFIG_HH
