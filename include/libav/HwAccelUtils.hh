// SPDX-FileCopyrightText: 2022 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef LIBAV_HWACCEL_UTILS_HH
#define LIBAV_HWACCEL_UTILS_HH

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/pixfmt.h>
}

#include <string>
#include <vector>

namespace vivictpp::libav {

AVPixelFormat selectSwPixelFormat(AVBufferRef *hwFramesCtx);
bool isHwAccelFormat(AVPixelFormat pixelFormat);
std::vector<std::string> allHwAccelFormats();

}; // namespace vivictpp::libav

#endif // LIBAV_HWACCEL_UTILS_HH
