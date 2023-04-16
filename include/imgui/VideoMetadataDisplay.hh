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
  int metadataVersion{-1};
  ImVec2 pos{10,10};
  std::vector<std::pair<std::string,std::string>> metadataText;
  std::vector<std::pair<std::string,std::string>> frameMetadataText;
  void initMetadataText(const ui::DisplayState &displayState);
  void initFrameMetadataText(const ui::DisplayState &displayState);
public:
  VideoMetadataDisplay(Type type):
    type(type) {};
  void draw(const ui::DisplayState &displayState);
};


}


#endif /* VIVICTPP_IMGUI_VIDEOMETADATADISPLAY_HH_ */
