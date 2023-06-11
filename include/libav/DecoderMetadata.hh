// SPDX-FileCopyrightText: 2023 Gustav Grusell (gustav.grusell@gmail.com)
//
// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef VIVICTPP_LIBAV_DECODERMETADATA_HH_
#define VIVICTPP_LIBAV_DECODERMETADATA_HH_

#include <string>

namespace vivictpp::libav {

struct DecoderMetadata {
  std::string name;
  std::string hwAccel{"none"};
  std::string hwPixelFormat;
};

}

#endif /* VIVICTPP_LIBAV_DECODERMETADATA_HH_ */
