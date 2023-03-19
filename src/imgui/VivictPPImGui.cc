#include "imgui/VivictPPImGui.hh"

#include "SDL_video.h"
#include "imgui.h"
#include "imgui/Events.hh"
#include "imgui_internal.h"
#include "VivictPPConfig.hh"
#include "sdl/SDLUtils.hh"
#include "ui/DisplayState.hh"
#include <memory>

// https://github.com/ocornut/imgui/issues/3379`
bool scrollWhenDraggingOnVoid(const ImVec2& delta, ImGuiMouseButton mouse_button)
{
  // blow
  ImGuiContext& g = *ImGui::GetCurrentContext();
  ImGuiWindow* window = g.CurrentWindow;
  ImGuiID id = window->GetID("##scrolldraggingoverlay");
  ImGui::KeepAliveID(id);
  bool hovered = false;
  bool held = false;
  bool scrolled = false;
  ImGuiButtonFlags button_flags = (mouse_button == 0) ? ImGuiButtonFlags_MouseButtonLeft : (mouse_button == 1) ? ImGuiButtonFlags_MouseButtonRight : ImGuiButtonFlags_MouseButtonMiddle;
  if (g.HoveredId == 0) // If nothing hovered so far in the frame (not same as IsAnyItemHovered()!)
    ImGui::ButtonBehavior(window->Rect(), id, &hovered, &held, button_flags);
  if (held && delta.x != 0.0f) {
    ImGui::SetScrollX(window, window->Scroll.x - delta.x);
    scrolled = true;
  }
  if (held && delta.y != 0.0f) {
    ImGui::SetScrollY(window, window->Scroll.y - delta.y);
    scrolled = true;
  }
  return scrolled;
}

void vivictpp::imgui::VideoWindow::onZoomChange(const Resolution &nativeResolution, const ui::Zoom &zoom) {
  ImVec2 oldVideoSize = videoSize;

  videoSize = {nativeResolution.w * zoom.multiplier(),
    nativeResolution.h * zoom.multiplier()};
  if (videoSize.x > size.x ||
      videoSize.y > size.y) {
    // Adjust scroll so center of picture stays the same
    // TODO: Take 'pad' into account maybe
    ImVec2 center = { (scroll.x + size.x / 2) / oldVideoSize.x,
      (scroll.y + size.y / 2) / oldVideoSize.y };
    scroll = { center.x * videoSize.x - size.x / 2,
      center.y * videoSize.y - size.y / 2 };
    scrollUpdated = true;
  } else {
    if (scroll.x != 0.0 || scroll.y != 0.0) {
      scroll = { 0, 0};
      scrollUpdated = true;
    }
  }
}

void vivictpp::imgui::VideoWindow::draw(vivictpp::ui::VideoTextures &videoTextures, const ui::DisplayState &displayState) {
  const ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImVec2 work_size = viewport->WorkSize;

  pos = {0,0};
  size = work_size;
  ImGui::SetNextWindowPos(pos);
  ImGui::SetNextWindowSize(size);

  float scaleFactor = displayState.zoom.multiplier();

  ImVec2 scaledVideoSize = {videoTextures.nativeResolution.w * displayState.zoom.multiplier() , videoTextures.nativeResolution.h * scaleFactor};
  videoSize = scaledVideoSize;
  ImGui::SetNextWindowContentSize({std::max(work_size.x, scaledVideoSize.x), std::max(work_size.y, scaledVideoSize.y)});
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

  bool myBool2;
  if (ImGui::Begin("Vivict++", &myBool2,  ImGuiWindowFlags_AlwaysAutoResize |
                   ImGuiWindowFlags_NoDecoration |
                   ImGuiWindowFlags_NoSavedSettings |
                   ImGuiWindowFlags_NoFocusOnAppearing |
                   ImGuiWindowFlags_NoNav |
                   ImGuiWindowFlags_NoBringToFrontOnFocus |
                   ImGuiWindowFlags_NoTitleBar |
                   ImGuiWindowFlags_NoScrollbar
        )) {
    ImVec2 viewSize = work_size;

    if (scrollUpdated) {
      ImGui::SetScrollX(scroll.x);
      ImGui::SetScrollY(scroll.y);
      scrollUpdated = false;
    } else {
      scroll = {ImGui::GetScrollX(), ImGui::GetScrollY()};
    }
    float scrollX = scroll.x;
    float scrollY = scroll.y;

    ImVec2 pad = ImGui::GetCursorPos();
    if (scaledVideoSize.x < viewSize.x) {
      pad.x += (viewSize.x - scaledVideoSize.x) / 2;
    }
    if (scaledVideoSize.y < viewSize.y) {
      pad.y += (viewSize.y - scaledVideoSize.y) / 2;
    }
    videoPos = pad;

    float splitFraction = displayState.splitPercent / 100;
    float splitX = pad.x + splitFraction * viewSize.x;
    ImVec2 drawPos = {pad.x - scrollX, pad.y - scrollY}; //cursorPos;
    ImVec2 uvMin(0, 0);

    ImVec2 uvMax((scrollX + splitX - pad.x) / scaledVideoSize.x ,1);
    ImVec2 p2(pad.x + scaledVideoSize.x - scrollX, pad.y + scaledVideoSize.y - scrollY);
    ImGui::GetWindowDrawList()->AddImage((void*)(intptr_t) videoTextures.leftTexture.get(),
                                         drawPos, p2 /*, uvMin, uvMax*/);

    if (!displayState.splitScreenDisabled) {
      // TODO: Should take into account size of video texture here
      drawPos.x = splitX;
      uvMin.x = uvMax.x;
      uvMax.x = 1.0; //(scrollX + viewSize.x) / scaledVideoSize.x;
      ImGui::GetWindowDrawList()->AddImage((void*)(intptr_t) videoTextures.rightTexture.get(),
                                           drawPos, p2, uvMin, uvMax);
      ImGui::GetWindowDrawList()->AddLine({splitX, pad.y}, {splitX, pad.y + scaledVideoSize.y}, 0x80FFFFFF, 0.5);
    }
    /*
      if (ImGui::InvisibleButton("##playpause", videoWindow.size)) {
      videoPlayback.togglePlaying();
      };
    */
    ImVec2 mouse_delta = ImGui::GetIO().MouseDelta;
    if (scrollWhenDraggingOnVoid(mouse_delta, 0)) {
      scroll = {ImGui::GetScrollX(), ImGui::GetScrollY()};
    }
  }
  ImGui::End();
  ImGui::PopStyleVar(2);
}

std::vector<vivictpp::imgui::Action>  vivictpp::imgui::Controls::draw(const PlaybackState &playbackState) {
  ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration |
                                  ImGuiWindowFlags_NoTitleBar |
                                  ImGuiWindowFlags_AlwaysAutoResize |
                                  ImGuiWindowFlags_NoSavedSettings |
                                  ImGuiWindowFlags_NoFocusOnAppearing |
                                  ImGuiWindowFlags_NoNav;
  const ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImVec2 work_size = viewport->WorkSize;
  ImVec2 work_pos = viewport->WorkPos;
  ImVec2 window_pos, window_pos_pivot;
  window_pos.x = work_pos.x + work_size.x / 2;
  window_pos.y = work_size.y - 10.0f;
  window_pos_pivot.x = 0.5f;
  window_pos_pivot.y = 1.0f;
  ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
  ImGui::SetNextWindowSize({work_size.x - 60, 60});
  window_flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground;
  ImGui::SetNextWindowBgAlpha(0.10f); // Transparent background
  std::vector<Action> actions;
  bool myBool;
  if (ImGui::Begin("Video controls", &myBool, window_flags)) {
    if (ImGui::IsWindowHovered()) {
      showControls = 70;
    } else if (showControls > 0) {
      showControls--;
    }
    if (showControls > 0) {
      ImGui::PushStyleVar(ImGuiStyleVar_Alpha, std::clamp(showControls / 60.0f, 0.0f, 1.0f));
      if (ImGui::Button(playbackState.playing ? "Pause" : "Play")) {
        actions.push_back({ActionType::PlayPause});
      }
      ImGui::SameLine();
      if (ImGui::Button("Step")) {
        /*
          if (videoInputs.ptsInRange(videoInputs.nextPts())) {
          pts = videoInputs.nextPts();
          //                  spdlog::info("Stepping to {}", pts);
          videoInputs.stepForward(pts);
          }
        */
      }
      ImGui::SameLine();
      if (ImGui::Button("Fullscreen")) {
        actions.push_back({ActionType::ToggleFullscreen});
      }
      ImGui::SameLine();
      if (ImGui::Button("Zoom in")) {
        actions.push_back({ActionType::ZoomIn});
      }
      ImGui::SameLine();
      if (ImGui::Button("Zoom out")) {
        actions.push_back({ActionType::ZoomOut});
      }
      ImGui::SameLine();
      if (ImGui::Button("Reset zoom")) {
        actions.push_back({ActionType::ZoomReset});
      }
      ImGui::PushItemWidth(work_size.x - 60);
      //float seekValue = playbackState.pts / 1e6; // Convert to seconds;
      float durationSeconds = playbackState.duration / 1e6;
      ImGui::SliderFloat("##Seekbar", &seekValue, 0.0f,  durationSeconds);
      float oldSeekValue = seekValue;
      ImGui::PopItemWidth();
      if (ImGui::IsItemActive()) {
        showControls = 70;
      } else if (!playbackState.seeking && !ImGui::IsItemDeactivatedAfterEdit()) {
        seekValue = playbackState.pts / 1e6; // Convert to seconds;
      }
      if (ImGui::IsItemActivated()) {
        spdlog::info("Drag start");
      }

      if (ImGui::IsItemDeactivatedAfterEdit()) {
        spdlog::info("Drag end: {}", seekValue);
        vivictpp::time::Time seekPos = (uint64_t) 1e6 * oldSeekValue;
        actions.push_back({ActionType::Seek, seekPos});
      }
      ImGui::PopStyleVar();
    }
  }
  ImGui::End();
  return actions;
}


vivictpp::imgui::VivictPPImGui::VivictPPImGui(VivictPPConfig vivictPPConfig):
  imGuiSDL(),
  videoPlayback(vivictPPConfig)
{
  displayState.splitScreenDisabled = vivictPPConfig.sourceConfigs.size() == 1;
  if (displayState.splitScreenDisabled) {
    displayState.splitPercent = 100;
  }
  displayState.updateFrames(videoPlayback.getVideoInputs().firstFrames());
  displayState.updateMetadata(videoPlayback.getVideoInputs().metadata());
  imGuiSDL.updateTextures(displayState);
  imGuiSDL.fitWindowToTextures();

}

void vivictpp::imgui::VivictPPImGui::run() {

  while (!done) {
    handleActions(handleEvents(imGuiSDL.handleEvents()));

    imGuiSDL.newFrame();

    int64_t tNextPresent = tLastPresent + (int64_t) (1e6 * ImGui::GetIO().DeltaTime);
    if (videoPlayback.checkAdvanceFrame(tNextPresent)) {
      displayState.updateFrames(videoPlayback.getVideoInputs().firstFrames());
      imGuiSDL.updateTextures(displayState);
    }

    videoWindow.draw(imGuiSDL.getVideoTextures(), displayState);

    handleActions(controls.draw(videoPlayback.getPlaybackState()));
    imGuiSDL.render();
    tLastPresent = vivictpp::time::relativeTimeMicros();
  }
}

int seekDistance(const vivictpp::imgui::KeyEvent &keyEvent) {
  return keyEvent.shift ? (keyEvent.alt ? 600 : 60) : 5;
}

vivictpp::imgui::Action vivictpp::imgui::VivictPPImGui::handleKeyEvent(const vivictpp::imgui::KeyEvent &keyEvent) {
  const std::string &key = keyEvent.keyName;
  if (key.length() == 1) {
    switch (key[0]) {
    case 'Q':
      return {vivictpp::imgui::ActionType::ActionQuit};
    case '.':
      if (keyEvent.shift) {
        return {vivictpp::imgui::ActionType::FrameOffsetIncrease};
      } else {
        return {vivictpp::imgui::ActionType::StepForward};
      }
      break;
    case ',':
      if (keyEvent.shift) {
        return {vivictpp::imgui::ActionType::FrameOffsetDecrease};
      } else {
        return {vivictpp::imgui::ActionType::StepBackward};
      }
    case '/':
      return {vivictpp::imgui::ActionType::SeekRelative,
        vivictpp::time::seconds(seekDistance(keyEvent))};
    case 'M':
      return {vivictpp::imgui::ActionType::SeekRelative,
        vivictpp::time::seconds(-1 * seekDistance(keyEvent))};
    case 'U':
      return {vivictpp::imgui::ZoomIn};
    case 'I':
      return {vivictpp::imgui::ZoomOut};
    case '0':
      return {vivictpp::imgui::ZoomReset};
    case 'F':
      return {vivictpp::imgui::ToggleFullscreen};
    case 'T':
      return {vivictpp::imgui::ToggleDisplayTime};
    case 'D':
      return {vivictpp::imgui::ToggleDisplayMetadata};
    case 'P':
      return {vivictpp::imgui::ToggleDisplayPlot};
    case 'S':
      return {vivictpp::imgui::ToggleFitToScreen};
    case '[':
      return {vivictpp::imgui::PlaybackSpeedIncrease};
    case ']':
      return {vivictpp::imgui::PlaybackSpeedDecrease};
    }
  } else {
    if (key == "Space") {
      return {vivictpp::imgui::PlayPause};
    }
  }
  return {vivictpp::imgui::NoAction};
}

std::vector<vivictpp::imgui::Action> vivictpp::imgui::VivictPPImGui::handleEvents(std::vector<std::shared_ptr<vivictpp::imgui::Event>> events) {
  std::vector<Action> actions;
  for (auto &event : events) {
    if (std::dynamic_pointer_cast<vivictpp::imgui::Quit>(event)) {
      actions.push_back({vivictpp::imgui::ActionQuit});
    } else if (std::dynamic_pointer_cast<vivictpp::imgui::WindowSizeChange>(event)) {
      // Do nothing
    } else  if (auto mouseMotion = std::dynamic_pointer_cast<vivictpp::imgui::MouseMotion>(event)) {
      if (!displayState.splitScreenDisabled) {
          displayState.splitPercent = 100.0 * std::clamp((mouseMotion->x - videoWindow.getVideoPos().x) / videoWindow.getVideoSize().x, 0.0f, 1.0f);
      }
    } else if (auto keyEvent = std::dynamic_pointer_cast<vivictpp::imgui::KeyEvent>(event)) {
      actions.push_back(handleKeyEvent(*keyEvent.get()));
    }
  }
  return actions;
}

void vivictpp::imgui::VivictPPImGui::handleActions(std::vector<vivictpp::imgui::Action> actions) {
  for (auto &action : actions) {
      switch (action.type) {
      case ActionType::ActionQuit:
        done = true;
        break;
      case ActionType::PlayPause:
        videoPlayback.togglePlaying();
        break;
      case ActionType::ZoomIn:
        displayState.zoom.increment();
        videoWindow.onZoomChange(imGuiSDL.getVideoTextures().nativeResolution, displayState.zoom);
        break;
      case ActionType::ZoomOut:
        displayState.zoom.decrement();
        videoWindow.onZoomChange(imGuiSDL.getVideoTextures().nativeResolution, displayState.zoom);
        break;
      case ActionType::ZoomReset:
        displayState.zoom.set(0);
        videoWindow.onZoomChange(imGuiSDL.getVideoTextures().nativeResolution, displayState.zoom);
        break;
      case ActionType::Seek:
        videoPlayback.seek(action.seek);
        break;
      case ActionType::SeekRelative:
        videoPlayback.seekRelative(action.seek);
        break;
      case ActionType::StepForward:
        videoPlayback.seekRelativeFrame(1);
        break;
      case ActionType::StepBackward:
        videoPlayback.seekRelativeFrame(-1);
        break;
      case ActionType::ToggleFullscreen:
        imGuiSDL.toggleFullscreen();
        break;
      default:
        ;
      }
    }
}