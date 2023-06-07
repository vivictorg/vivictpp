// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "imgui/Help.hh"

#include "imgui.h"

const std::string HELP_TEXT =
  R"(
KEYBOARD SHORTCUTS

SPACE  Play/Pause video
,      Step forward 1 frame
.      Step backward 1 frame
/ or - Seek forward 5 seconds
m      Seek backward 5 seconds
?      Seek Forward 60s
M      Seek backward 60s
Alt-?  Seek Forward 10min
Alt-M  Seek Backward 10min
<      Decrease left frame offset
>      Increase left frame offset
[      Decrease playback speed
]      Increase playback speed

f      Toggle full screen
u      Zoom in
i      Zoom out
0      Reset pan and zoom to default
s      Toggle scale content to fit window
t      Toggle visibility of time
d      Toggle visibility of Stream and Frame metadata

q      Quit application)";

void vivictpp::imgui::showHelp(ui::DisplayState &displayState) {
  if (ImGui::Begin("Help", &displayState.displayHelp)) {
    ImGui::Text("%s", HELP_TEXT.c_str());
  }
  ImGui::End();
}
