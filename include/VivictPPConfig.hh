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
  VivictPPConfig(std::vector<SourceConfig> sourceConfigs, bool disableAudio):
    sourceConfigs(sourceConfigs),
    disableAudio(disableAudio) {}

  const std::vector<SourceConfig> sourceConfigs;

  const bool disableAudio;

public:
  bool hasVmafData() {
    return std::any_of(sourceConfigs.begin(),
                sourceConfigs.end(),
                [](const SourceConfig &sc) { return !sc.vmafLog.empty(); });
  }
};

#endif // VIVICTPPCONFIG_HH
