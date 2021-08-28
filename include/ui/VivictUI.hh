// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIVICT_UI_HH_
#define VIVICT_UI_HH_

#include "VideoMetadata.hh"
#include "ui/DisplayState.hh"
#include "libav/Frame.hh"

namespace vivictpp {
namespace ui {

class VivictUI {
 public:
  virtual ~VivictUI() = default;
  virtual void displayFrame(const std::array<vivictpp::libav::Frame, 2> &frames,
                            const DisplayState &displayState) = 0;
  virtual int getWidth() = 0;
  virtual int getHeight() = 0;
  virtual void setFullscreen(bool fullscreen) = 0;
  virtual void setCursorHand() = 0;
  virtual void setCursorDefault() = 0;
  virtual void setLeftMetadata(const VideoMetadata &metadata) = 0;
  virtual void setRightMetadata(const VideoMetadata &metadata) = 0;
};

};  // ui
};  // vivictpp

#endif  // VIVICT_UI_HH_
