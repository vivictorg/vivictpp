#ifndef VIVICTPP_IMGUI_CONTROLS_HH_
#define VIVICTPP_IMGUI_CONTROLS_HH_

#include <vector>
#include "imgui.h"
#include "imgui/Events.hh"
#include "VideoPlayback.hh"
#include "imgui/VideoMetadataDisplay.hh"

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
                           const ui::DisplayState &displayState);
};

}

#endif /* VIVICTPP_IMGUI_CONTROLS_HH_ */
