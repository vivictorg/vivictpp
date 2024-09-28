// SPDX-FileCopyrightText: 2024 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIVICTPP_VIDEOWINDOW_HH
#define VIVICTPP_VIDEOWINDOW_HH

#include "Settings.hh"
#include "VideoPlayback.hh"
#include "VivictPPConfig.hh"
#include "imgui.h"
#include "imgui/Controls.hh"
#include "imgui/Events.hh"
#include "imgui/FileDialog.hh"
#include "imgui/ImGuiSDL.hh"
#include "imgui/MainMenu.hh"
#include "imgui/SettingsDialog.hh"
#include "imgui/VideoMetadataDisplay.hh"
#include "sdl/SDLUtils.hh"
#include "ui/DisplayState.hh"
#include "ui/VideoTextures.hh"
#include <vector>
class VideoWindow {
private:
  ImVec2 pos;
  ImVec2 scroll;
  ImVec2 size;
  ImVec2 videoSize;
  ImVec2 videoPos;
  bool scrollUpdated{false};
  bool wasDragging{false};

public:
  void draw(vivictpp::ui::VideoTextures &videoTextures,
            const vivictpp::ui::DisplayState &displayState);
  void onZoomChange(const Resolution &nativeResolution,
                    const vivictpp::ui::Zoom &zoom);
  void onScroll(const ImVec2 &scrollDelta);
  const ImVec2 &getVideoPos() { return videoPos; }
  const ImVec2 &getVideoSize() { return videoSize; }
};
#endif // VIVICTPP_VIDEOWINDOW_HH
