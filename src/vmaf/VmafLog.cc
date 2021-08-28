// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "vmaf/VmafLog.hh"

vivictpp::vmaf::VmafLog::VmafLog(std::string logfile) {
  if (!logfile.empty()) {
    std::ifstream infile(logfile);
    std::string line;
    std::getline(infile, line);
    std::map<std::string, int> headers;
    getHeaders(line, headers);
    while(std::getline(infile, line)) {
      auto values = parseLine(line);
      vmafValues.push_back(values[headers["vmaf"]]);
    }
  }
}


void vivictpp::vmaf::VmafLog::getHeaders(std::string line,
                                         std::map<std::string, int> & headers) {
  std::stringstream ss(line);
  std::string header;
  int i = 0;
  while (ss.good()) {
    getline(ss, header, ',');
    headers[header] = i++;
    if (ss.peek() == ',')
      ss.ignore();
  }
}

std::vector<float> vivictpp::vmaf::VmafLog::parseLine(std::string line) {
  std::vector<float> result;
  std::stringstream ss(line);
  float value;
  for (int i = 0; ss >> value; i++) {
    result.push_back(value);
    if (ss.peek() == ',')
      ss.ignore();
  }
  return result;
}
