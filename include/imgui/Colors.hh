#ifndef VIVICTPP_IMGUI_COLORS_HH_
#define VIVICTPP_IMGUI_COLORS_HH_

#include "imgui.h"

namespace vivictpp::imgui {
inline ImU32 transparentBg = ImGui::ColorConvertFloat4ToU32({0.0f, 0.0f, 0.0f, 0.4f});

inline ImU32 transparent = ImGui::ColorConvertFloat4ToU32({0.0f, 0.0f, 0.0f, 0.0f});
}


#endif /* VIVICTPP_IMGUI_COLORS_HH_ */
