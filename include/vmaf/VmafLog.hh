// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VMAF_VMAFLOG_HH
#define VMAF_VMAFLOG_HH

#include <fstream>
#include <map>
#include <ostream>
#include <sstream>
#include <vector>

namespace vivictpp {
namespace vmaf {

class VmafLog {
private:
  std::vector<float> vmafValues;

public:
  VmafLog(std::string logfile);

  const std::vector<float> &getVmafValues() const { return vmafValues; }

  bool empty() const { return vmafValues.empty(); }

private:
  void getHeaders(std::string line, std::map<std::string, int> &headers);
  std::vector<float> parseLine(std::string line);
};

} // namespace vmaf
} // namespace vivictpp

#endif // VMAF_VMAFLOG_HH
