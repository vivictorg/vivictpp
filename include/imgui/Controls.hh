// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIVICTPP_IMGUI_CONTROLS_HH_
#define VIVICTPP_IMGUI_CONTROLS_HH_

#include <vector>
#include "imgui.h"
#include "imgui/Events.hh"
#include "VideoPlayback.hh"
#include "imgui/VideoMetadataDisplay.hh"
#include "ui/ThumbnailTexture.hh"

namespace vivictpp::imgui {

class Controls {
private:
  int showControls{70};
  float seekValue{0};
  bool wasDragging{false};
  VideoMetadataDisplay leftMetadata{VideoMetadataDisplay::Type::LEFT};
  VideoMetadataDisplay rightMetadata{VideoMetadataDisplay::Type::RIGHT};

public:
  std::vector<Action> draw(const PlaybackState &playbackState,
                           const ui::DisplayState &displayState,
                           vivictpp::ui::ThumbnailTexture &thumbnailTexture);
};

}

#endif /* VIVICTPP_IMGUI_CONTROLS_HH_ */
