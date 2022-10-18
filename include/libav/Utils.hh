// SPDX-FileCopyrightText: 2022 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef LIBAV_UTILS_HH
#define LIBAV_UTILS_HH

#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

namespace vivictpp::libav {

int getChannels(AVCodecContext *codecContext);

int getChannels(AVFrame *frame);

std::string getChannelLayout(AVCodecContext *codecContext);

}  // namespace vivictpp::libav

#endif  // LIBAV_UTILS_HH
