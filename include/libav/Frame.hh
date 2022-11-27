// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef LIBAV_FRAME_HH
#define LIBAV_FRAME_HH

#include <memory>

extern "C" {
#include <libavutil/avutil.h>
#include <libavutil/frame.h>
}

#include "VideoMetadata.hh"

namespace vivictpp::libav {

class Frame {
private:
  std::shared_ptr<AVFrame> frame;
public:
  Frame();
  Frame(const Frame &frame);
  ~Frame() = default;
  Frame &operator=(const Frame &frame);
  AVFrame* avFrame() const { return frame.get(); }
  bool empty() const { return !frame; }
  void reset();
  Frame static emptyFrame() { return Frame(nullptr); }
  int64_t pts() const {
    if (frame) {
      return frame->best_effort_timestamp;
    } else {
      return AV_NOPTS_VALUE;
    }
  }
  FrameMetadata metadata() const {
    if (frame) {
      return { av_get_picture_type_char(frame->pict_type), frame->pts, frame->pkt_size };
    } else {
      return { '?', 0, 0 };
    }
  }
  Frame transferHwData(AVPixelFormat swPixelFormat);
private:
  Frame(AVFrame *avFrame);
};

void freeFrame(AVFrame* avFrame);

}  // namespace vivictpp::libav

#endif // LIBAV_FRAME_HH
