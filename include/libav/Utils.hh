// SPDX-FileCopyrightText: 2022 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef LIBAV_UTILS_HH
#define LIBAV_UTILS_HH

#include <string>
#include <vector>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

namespace vivictpp::libav {


#if LIBAVCODEC_VERSION_MAJOR >= 60
    // Data to be passed from packet to frames through opaque_ref field
    struct FrameData {
        int pktSize;
    };
#endif

int isKeyFrame(const AVFrame *frame);

void setOpaqueRef(AVPacket *pkt);

int getPacketSize(const AVFrame *frame);

int getChannels(AVCodecContext *codecContext);

int getChannels(AVFrame *frame);

std::string getChannelLayout(AVCodecContext *codecContext);

std::vector<std::string> allVideoDecoders();

}  // namespace vivictpp::libav

#endif  // LIBAV_UTILS_HH
