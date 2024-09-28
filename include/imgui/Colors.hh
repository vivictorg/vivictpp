// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIVICTPP_IMGUI_COLORS_HH_
#define VIVICTPP_IMGUI_COLORS_HH_

#include "imgui.h"

namespace vivictpp::imgui {
inline ImU32 transparentBg =
    ImGui::ColorConvertFloat4ToU32({0.0f, 0.0f, 0.0f, 0.4f});

inline ImU32 transparent =
    ImGui::ColorConvertFloat4ToU32({0.0f, 0.0f, 0.0f, 0.0f});

inline ImU32 border = ImGui::ColorConvertFloat4ToU32({0.7f, 0.7f, 0.7f, 0.5f});
} // namespace vivictpp::imgui

#endif /* VIVICTPP_IMGUI_COLORS_HH_ */
