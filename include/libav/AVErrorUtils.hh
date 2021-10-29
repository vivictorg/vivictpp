// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef AV_ERROR_UTILS_HH_
#define AV_ERROR_UTILS_HH_

#include <string>

#include "libavutil/error.h"

namespace vivictpp {
namespace libav {

class AVResult {
public:
  AVResult():
    code(0) { }
  AVResult(int code):
    code(code) {  }

  std::string getMessage() {
    char buf[512];
    av_strerror(code, buf, 512);
    return std::string(buf);
  }
  bool operator==(const AVResult &other) {
    return this->code == other.code;
  }
  bool success() { return code == 0; }
  bool error() { return code != 0; }
  bool eagain() { return code == AVERROR(EAGAIN); }
  bool eof() { return code == AVERROR_EOF; }

private:
  int code;
};

}  // namespace libav
}  // namespace vivictpp

#endif  // AV_ERROR_UTILS_HH_
