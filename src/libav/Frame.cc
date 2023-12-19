// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "libav/Frame.hh"

#include "libav/AVErrorUtils.hh"

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

vivictpp::libav::Frame::Frame(const Frame &otherFrame) {
    updatedFilteredMetadata = otherFrame.updatedFilteredMetadata;
  if (otherFrame.frame) {
    frame.reset(av_frame_clone(otherFrame.frame.get()), &freeFrame);
  } else {
    frame.reset((AVFrame*) nullptr, &freeFrame);
  }
}

vivictpp::libav::Frame &vivictpp::libav::Frame::operator=(const Frame &otherFrame) {
    updatedFilteredMetadata = otherFrame.updatedFilteredMetadata;
  if (otherFrame.frame) {
    frame.reset(av_frame_clone(otherFrame.frame.get()), &freeFrame);
  } else {
    frame.reset((AVFrame*) nullptr, &freeFrame);
  }
  return *this;
}

vivictpp::libav::Frame vivictpp::libav::Frame::transferHwData(AVPixelFormat swPixelFormat) {
  // av_hwframe_transfer_get_formats(src, AV_HWFRAME_TRANSFER_DIRECTION_FROM)

  
  Frame swFrame;
  swFrame.avFrame()->format = swPixelFormat;
  AVResult ret = av_hwframe_transfer_data(swFrame.avFrame(), avFrame(), 0);
  if (ret.error()) {
    throw std::runtime_error("Failed to transfer hw frame");
  }
  ret = av_frame_copy_props(swFrame.avFrame(), avFrame());
  if (ret.error()) {
    throw std::runtime_error("Failed to copy frame props");
  }
  return swFrame;
}

/*
  Frame(const Frame &frame);
  ~Frame() = default;
  Frame &operator=(const Frame &frame);
 */

void vivictpp::libav::Frame::reset() {
    updatedFilteredMetadata.reset();
  frame.reset(av_frame_alloc(), &freeFrame);
}
