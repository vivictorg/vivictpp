// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIVICTPP_UI_VIDEOTEXTURES_HH_
#define VIVICTPP_UI_VIDEOTEXTURES_HH_

#include "Resolution.hh"
#include "imgui.h"
#include "ui/DisplayState.hh"

namespace vivictpp::ui {

struct VideoTextures {
  ImTextureID leftTexture;
  ImTextureID rightTexture;
  Resolution nativeResolution;
  AVRational nativeAspectRatio;

public:
  void calcNativeResolution(const vivictpp::ui::DisplayState &displayState);
};
}; // namespace vivictpp::ui

#endif /* VIVICTPP_UI_VIDEOTEXTURES_HH_ */
