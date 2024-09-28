// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIVICTPP_IMGUI_MAINMENU_HH_
#define VIVICTPP_IMGUI_MAINMENU_HH_

#include "Events.hh"
#include "VideoPlayback.hh"
#include "ui/DisplayState.hh"
#include <vector>

namespace vivictpp::imgui {

class MainMenu {
public:
  std::vector<Action> draw(const PlaybackState &playbackState,
                           const ui::DisplayState &displayState);
};

} // namespace vivictpp::imgui

#endif /* VIVICTPP_IMGUI_MAINMENU_HH_ */
