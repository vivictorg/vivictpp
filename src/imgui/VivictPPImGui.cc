#include "imgui/VivictPPImGui.hh"

#include "SDL_video.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "VivictPPConfig.hh"
#include "sdl/SDLUtils.hh"
#include "ui/DisplayState.hh"

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

std::vector<vivictpp::imgui::Event>  vivictpp::imgui::Controls::draw(const PlaybackState &playbackState) {
  ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration |
                                  ImGuiWindowFlags_NoTitleBar |
                                  ImGuiWindowFlags_AlwaysAutoResize |
                                  ImGuiWindowFlags_NoSavedSettings |
                                  ImGuiWindowFlags_NoFocusOnAppearing |
                                  ImGuiWindowFlags_NoNav;
  const ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImVec2 work_size = viewport->WorkSize;
  std::vector<Event> events;
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
        events.push_back({EventType::PlayPause, {0}});
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
        events.push_back({EventType::ToggleFullscreen, {0}});
      }
      ImGui::SameLine();
      if (ImGui::Button("Zoom in")) {
        events.push_back({EventType::ZoomIn, {0}});
      }
      ImGui::SameLine();
      if (ImGui::Button("Zoom out")) {
        events.push_back({EventType::ZoomOut, {0}});
      }
      ImGui::SameLine();
      if (ImGui::Button("Reset zoom")) {
        events.push_back({EventType::ZoomReset, {0}});
      }
      ImGui::PushItemWidth(work_size.x - 60);
      float seekValue = playbackState.pts / 1e6; // Convert to seconds;
      float durationSeconds = playbackState.duration / 1e6;
      ImGui::SliderFloat("##Seekbar", &seekValue, 0.0f,  durationSeconds);
      ImGui::PopItemWidth();
      if (ImGui::IsItemActive()) {
        showControls = 70;
      }
      if (ImGui::IsItemActivated()) {
        spdlog::info("Drag start");
      }

      if (ImGui::IsItemDeactivatedAfterEdit()) {
        spdlog::info("Drag end: {}", seekValue);
        vivictpp::time::Time seekPos = (uint64_t) 1e6 * seekValue;
        events.push_back({EventType::Seek, {seekPos}});
      }
      ImGui::PopStyleVar();
    }
  }
  ImGui::End();
  return events;
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
    handleEvents(imGuiSDL.handleEvents());

    int64_t tNextPresent = tLastPresent + (int64_t) (1e6 * ImGui::GetIO().DeltaTime);
    if (videoPlayback.checkdvanceFrame(tNextPresent)) {
      displayState.updateFrames(videoPlayback.getVideoInputs().firstFrames());
      imGuiSDL.updateTextures(displayState);
    }

    videoWindow.draw(imGuiSDL.getVideoTextures(), displayState);

    handleEvents(controls.draw(videoPlayback.getPlaybackState()));
    imGuiSDL.render();
    tLastPresent = vivictpp::time::relativeTimeMicros();
  }
}



void vivictpp::imgui::VivictPPImGui::handleEvents(std::vector<vivictpp::imgui::Event> events) {
  for (auto &event : events) {
      switch (event.type) {
      case EventType::Quit:
        done = true;
        break;
      case EventType::WindowSizeChange:
        break;
      case EventType::MouseMotion:
        if (!displayState.splitScreenDisabled) {
          displayState.splitPercent = 100.0 * std::clamp((event.mouseMotion.x - videoWindow.getVideoPos().x) / videoWindow.getVideoSize().x, 0.0f, 1.0f);
        }
        break;
      case EventType::PlayPause:
        videoPlayback.togglePlaying();
        break;
      case EventType::ZoomIn:
        displayState.zoom.increment();
        videoWindow.onZoomChange(imGuiSDL.getVideoTextures().nativeResolution, displayState.zoom);
        break;
      case EventType::ZoomOut:
        displayState.zoom.decrement();
        videoWindow.onZoomChange(imGuiSDL.getVideoTextures().nativeResolution, displayState.zoom);
        break;
      case EventType::ZoomReset:
        displayState.zoom.set(0);
        videoWindow.onZoomChange(imGuiSDL.getVideoTextures().nativeResolution, displayState.zoom);
        break;
      case EventType::Seek:
        videoPlayback.seek(event.seek);
        break;
      case EventType::ToggleFullscreen:
        imGuiSDL.toggleFullscreen();
        break;
      default:
        ;
      }
    }
}
