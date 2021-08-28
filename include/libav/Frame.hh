// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FRAME_HH_
#define FRAME_HH_

#include <memory>


extern "C" {
#include <libavutil/avutil.h>
#include <libavutil/frame.h>
}

namespace vivictpp {
namespace libav {

class Frame {
private:
  std::shared_ptr<AVFrame> frame;
public:
  Frame();
  ~Frame() = default;
  AVFrame* avFrame() const { return frame.get(); }
  bool empty() const { return !frame; }
  void reset();
  Frame static emptyFrame() { return Frame(nullptr); }
  int64_t pts() const { return (frame ? frame->best_effort_timestamp : AV_NOPTS_VALUE); }
private:
  Frame(AVFrame *avFrame);
};

void freeFrame(AVFrame* avFrame);

};  // namespace libav
};  // namespace vivictpp

#endif  // FRAME_HH_
