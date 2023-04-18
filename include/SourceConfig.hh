// SPDX-FileCopyrightText: 2020 Sveriges Television AB
// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef SOURCECONFIG_HH_
#define SOURCECONFIG_HH_

#include <string>
#include "vmaf/VmafLog.hh"
#include "libav/DecoderOptions.hh"

class SourceConfig {
public:
  SourceConfig(std::string path,
               std::string filter = "",
               std::string vmafLogFile = "",
               std::string formatOptions = ""):
    path(path),
    filter(filter),
    vmafLog(vmafLogFile),
    formatOptions(formatOptions)
    {
    }

  const std::string path;
  const std::string filter;
  const vivictpp::vmaf::VmafLog vmafLog;
  const std::string formatOptions;
};

#endif  // SOURCECONFIG_HH_
