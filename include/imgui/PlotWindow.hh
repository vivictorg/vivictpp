// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIVICTPP_IMGUI_PLOTWINDOW_HH_
#define VIVICTPP_IMGUI_PLOTWINDOW_HH_

#include "VideoPlayback.hh"
#include "imgui.h"
#include "imgui/Events.hh"
#include "imgui/VideoMetadataDisplay.hh"
#include "ui/ThumbnailTexture.hh"
#include <vector>

namespace vivictpp::imgui {

class PlotWindow {
private:
  double timeX[2] = {0.0, 0.0};
  double timeY[2] = {0.0, 0.0};
  std::shared_ptr<vivictpp::video::VideoIndex> leftVideoIndex;
  std::shared_ptr<vivictpp::video::VideoIndex> rightVideoIndex;

public:
  PlotWindow(std::shared_ptr<vivictpp::video::VideoIndex> leftVideoIndex, std::shared_ptr<vivictpp::video::VideoIndex> rightVideoIndex)
      : leftVideoIndex(leftVideoIndex), rightVideoIndex(rightVideoIndex) {}
  std::vector<Action> draw(const ui::DisplayState &displayState);
};

} // namespace vivictpp::imgui

#endif /* VIVICTPP_IMGUI_PLOTWINDOW_HH_ */
