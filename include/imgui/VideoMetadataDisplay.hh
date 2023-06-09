// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIVICTPP_IMGUI_VIDEOMETADATADISPLAY_HH_
#define VIVICTPP_IMGUI_VIDEOMETADATADISPLAY_HH_

#include <string>
#include <vector>
#include "imgui.h"
#include "ui/DisplayState.hh"

namespace vivictpp::imgui {

class VideoMetadataDisplay {
public:
  enum Type {LEFT, RIGHT};
private:
  Type type;
  ImVec2 pos{10,10};
public:
  VideoMetadataDisplay(Type type):
    type(type) {};
  void draw(const ui::DisplayState &displayState);
  float calcWidth();
};


}


#endif /* VIVICTPP_IMGUI_VIDEOMETADATADISPLAY_HH_ */
