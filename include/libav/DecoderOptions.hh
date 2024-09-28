// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIVICTPP_LIBAV_DECODEROPTIONS_HH_
#define VIVICTPP_LIBAV_DECODEROPTIONS_HH_

#include <string>
#include <vector>

namespace vivictpp {
namespace libav {

struct DecoderOptions {
  std::vector<std::string> hwAccels;
  std::vector<std::string> preferredDecoders;
};

} // namespace libav
} // namespace vivictpp

#endif /* VIVICTPP_LIBAV_DECODEROPTIONS_HH_ */
