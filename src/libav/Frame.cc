// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "libav/Frame.hh"

void vivictpp::libav::freeFrame(AVFrame* avFrame) {
  if (avFrame) {
    av_frame_free(&avFrame);
  }
}

vivictpp::libav::Frame::Frame():
  frame(av_frame_alloc(), &freeFrame) {
}

vivictpp::libav::Frame::Frame(AVFrame *frame):
  frame(frame, &freeFrame) {
}

void vivictpp::libav::Frame::reset() {
  frame.reset(av_frame_alloc(), &freeFrame);
}
