// SPDX-FileCopyrightText: 2020 Sveriges Television AB
// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef SOURCECONFIG_HH_
#define SOURCECONFIG_HH_

#include <string>
#include <vector>
// #include "vmaf/VmafLog.hh"
#include "libav/DecoderOptions.hh"

class SourceConfig {
public:
  SourceConfig(std::string path, std::vector<std::string> hwAccels = {},
               std::vector<std::string> preferredDecoders = {},
               std::string filter = "",
               //               std::string vmafLogFile = "",
               std::string formatOptions = "")
      : path(path), hwAccels(hwAccels), preferredDecoders(preferredDecoders),
        filter(filter),
        //    vmafLog(vmafLogFile),
        formatOptions(formatOptions) {}

  std::string path;
  std::vector<std::string> hwAccels;
  std::vector<std::string> preferredDecoders;
  std::string filter;
  //  const vivictpp::vmaf::VmafLog vmafLog;
  std::string formatOptions;
};

#endif // SOURCECONFIG_HH_
