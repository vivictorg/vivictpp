#ifndef VIVICTPP_IMGUI_VIVICTPPIMGUI_HH_
#define VIVICTPP_IMGUI_VIVICTPPIMGUI_HH_

#include "VideoPlayback.hh"
#include "VivictPPConfig.hh"
#include "sdl/SDLUtils.hh"
#include "ui/DisplayState.hh"
#include "ui/VideoTextures.hh"
#include "imgui/Events.hh"
#include "imgui/ImGuiSDL.hh"
#include "imgui.h"
#include <vector>

namespace vivictpp::imgui {

class VideoWindow {
private:
  ImVec2 pos;
  ImVec2 scroll;
  ImVec2 size;
  ImVec2 videoSize;
  ImVec2 videoPos;
  bool scrollUpdated{false};
public:
  void draw(vivictpp::ui::VideoTextures &videoTextures, const ui::DisplayState &displayState);
  void onZoomChange(const Resolution &nativeResolution, const ui::Zoom &zoom);
  const ImVec2 &getVideoPos() { return videoPos; }
  const ImVec2 &getVideoSize() { return videoSize; }
};

class Controls {
private:
  int showControls{70};
public:
  std::vector<Event> draw(const PlaybackState &playbackState);
};

class VivictPPImGui {
private:
  ImGuiSDL imGuiSDL;
  VideoPlayback videoPlayback;
  bool done{false};
  ui::DisplayState displayState;
//  vivictpp::ui::VideoTextures videoTextures;
  VideoWindow videoWindow;
  Controls controls;
  int64_t tLastPresent{0};
private:
  void handleEvents(std::vector<vivictpp::imgui::Event> events);
public:
  VivictPPImGui(VivictPPConfig vivictPPConfig);
  void run();
};

}  // namespace vivictpp::imgui


#endif /* VIVICTPP_IMGUI_VIVICTPPIMGUI_HH_ */
