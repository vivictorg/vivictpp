// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIVICTPP_IMGUI_VIDEOMETADATADISPLAY_HH_
#define VIVICTPP_IMGUI_VIDEOMETADATADISPLAY_HH_

#include "imgui.h"
#include "ui/DisplayState.hh"
#include <string>
#include <vector>

namespace vivictpp::imgui {

class VideoMetadataDisplay {
public:
  enum Type { LEFT, RIGHT };

private:
  Type type;
  ImVec2 p1{10, 10};
  ImVec2 p2{100, 100};

public:
  VideoMetadataDisplay(Type type) : type(type){};
  void draw(const ui::DisplayState &displayState);
  float calcWidth();
};

} // namespace vivictpp::imgui

#endif /* VIVICTPP_IMGUI_VIDEOMETADATADISPLAY_HH_ */
