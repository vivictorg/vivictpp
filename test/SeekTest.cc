// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch2/catch.hpp"

#include "libav/FormatHandler.hh"

#include <unistd.h>
#include <memory>
#include <unistd.h>
#include <cmath>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include "libav/Decoder.hh"

void free_frame(AVFrame* frame) {
    av_frame_free(&frame);
}

double testSeek(std::string url, double pts) {
  /*
    char *cwd = get_current_dir_name();
    printf("pwd: %s\n", cwd);
    free(cwd);
    vivictpp::libav::FormatHandler formatHandler(url);
    AVStream *stream = formatHandler.getVideoStreams()[0];
    formatHandler.setStreamActive(stream->index);
    double seek_pts = pts;

    formatHandler.seek(seek_pts);

    AVPacket *pkt = nullptr;

    for(int i = 0; i < 10; i++) {
        pkt = formatHandler.nextPacket();
        if (pkt) {
            return ((double) pkt->dts) / stream->time_base.den;
        }
    }
    throw std::runtime_error("No package received");
  */
}

bool closeEnough(double a, double b) {
    return abs(a-b) < 0.00001;
}

//const std::string url("http://bitdash-a.akamaihd.net/content/sintel/hls/video/6000kbit.m3u8");
const std::string hls_playlist("../testdata/hls1/hls1.m3u8");
const std::string hls_playlist2("../testdata/hls2/master.m3u8");
const std::string mp4_file("../testdata/test1.mp4");

TEST_CASE( "Seek HLS seeks to first keyframe before given timestamp" "[FormatHandler.seek]") {
    REQUIRE(closeEnough(testSeek(hls_playlist, 1.4), 1.4));
    REQUIRE(closeEnough(testSeek(hls_playlist, 1.5), 1.4));
    REQUIRE(closeEnough(testSeek(hls_playlist, 6.3), 1.4));
    REQUIRE(closeEnough(testSeek(hls_playlist, 12), 6.4));

    REQUIRE(closeEnough(testSeek(hls_playlist2, 1.4), 1.423222));
    REQUIRE(closeEnough(testSeek(hls_playlist2, 1.5), 1.423222));
    REQUIRE(closeEnough(testSeek(hls_playlist2, 6.3), 1.423222));
    REQUIRE(closeEnough(testSeek(hls_playlist2, 12), 6.423222));
}

TEST_CASE( "Seek mp4 seeks to first keyframe before given timestamp" "[FormatHandler.seek]") {
    REQUIRE(testSeek(mp4_file, 0) == 0);
    REQUIRE(testSeek(mp4_file, 1.5) == 0);
    REQUIRE(testSeek(mp4_file, 4.9) == 0);
    REQUIRE(testSeek(mp4_file, 9) == 5);
}
