// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIVICTPP_IMGUI_FONTS_HH_
#define VIVICTPP_IMGUI_FONTS_HH_

#include "imgui.h"

#define ICON_VPP_COLLAPSE "\xef\x80\x89"	// U+f000
#define ICON_VPP_EXPAND "\xef\x80\x8a"
#define ICON_VPP_PLAY "\xef\x80\x8b"
#define ICON_VPP_PAUSE "\xef\x80\x8c"
#define ICON_VPP_STEP_BACKWARD "\xef\x80\x8d"
#define ICON_VPP_STEP_FORWARD "\xef\x80\x8e"
#define ICON_VPP_ZOOM_IN "\xef\x80\x8f"
#define ICON_VPP_ZOOM_OUT "\xef\x80\x90"
#define ICON_VPP_ZOOM_RESET "\xef\x80\x91"
#define ICON_VPP_COLLAPSE_SMALL "\xef\x80\x80"	// U+f000
#define ICON_VPP_EXPAND_SMALL "\xef\x80\x81"
#define ICON_VPP_PLAY_SMALL "\xef\x80\x82"
#define ICON_VPP_PAUSE_SMALL "\xef\x80\x83"
#define ICON_VPP_STEP_BACKWARD_SMALL "\xef\x80\x84"
#define ICON_VPP_STEP_FORWARD_SMALL "\xef\x80\x85"
#define ICON_VPP_ZOOM_IN_SMALL "\xef\x80\x86"
#define ICON_VPP_ZOOM_OUT_SMALL "\xef\x80\x87"
#define ICON_VPP_ZOOM_RESET_SMALL "\xef\x80\x88"


namespace vivictpp::imgui {

void initFonts();
ImFont* getIconFont();
}


#endif /* VIVICTPP_IMGUI_FONTS_HH_ */
