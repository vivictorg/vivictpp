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


class VideoMetadataDisplay {
public:
  enum Type {LEFT, RIGHT};
private:
  Type type;
  int metadataVersion{-1};
  ImVec2 pos{10,10};
  std::vector<std::pair<std::string,std::string>> metadataText;
  std::vector<std::pair<std::string,std::string>> frameMetadataText;
  void initMetadataText(const ui::DisplayState &displayState);
  void initFrameMetadataText(const ui::DisplayState &displayState);
public:
  VideoMetadataDisplay(Type type):
    type(type) {};
  void draw(const ui::DisplayState &displayState);
};

class VideoWindow {
private:
  ImVec2 pos;
  ImVec2 scroll;
  ImVec2 size;
  ImVec2 videoSize;
  ImVec2 videoPos;
  bool scrollUpdated{false};
  VideoMetadataDisplay leftMetadata{VideoMetadataDisplay::Type::LEFT};
  VideoMetadataDisplay rightMetadata{VideoMetadataDisplay::Type::RIGHT};
public:
  void draw(vivictpp::ui::VideoTextures &videoTextures, const ui::DisplayState &displayState);
  void onZoomChange(const Resolution &nativeResolution, const ui::Zoom &zoom);
  const ImVec2 &getVideoPos() { return videoPos; }
  const ImVec2 &getVideoSize() { return videoSize; }
};

class Controls {
private:
  int showControls{70};
  float seekValue{0};
public:
  std::vector<Action> draw(const PlaybackState &playbackState);
};

/*
class TimeDisplay {
public:
  void draw(const ui::DisplayState &displayState);
};
*/

class VivictPPImGui {
private:
  ImGuiSDL imGuiSDL;
  VideoPlayback videoPlayback;
  bool done{false};
  ui::DisplayState displayState;
  VideoWindow videoWindow;
  Controls controls;
  int64_t tLastPresent{0};
private:
  Action handleKeyEvent(const KeyEvent &keyEvent);
  std::vector<Action> handleEvents(std::vector<std::shared_ptr<vivictpp::imgui::Event>> events);
  void handleActions(std::vector<vivictpp::imgui::Action> actions);
public:
  VivictPPImGui(VivictPPConfig vivictPPConfig);
  void run();
};

}  // namespace vivictpp::imgui


#endif /* VIVICTPP_IMGUI_VIVICTPPIMGUI_HH_ */
