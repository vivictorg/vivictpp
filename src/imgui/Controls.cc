#include "imgui/Controls.hh"
#include "imgui/Colors.hh"
#include "imgui.h"

std::vector<vivictpp::imgui::Action>  vivictpp::imgui::Controls::draw(
  const PlaybackState &playbackState,
  const ui::DisplayState &displayState) {
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
  ImGui::SetNextWindowPos({0,0}, ImGuiCond_Always);
  ImGui::SetNextWindowSize({work_size.x, work_size.y});
  window_flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground;
  ImGui::SetNextWindowBgAlpha(0.0f); // Transparent background
  std::vector<Action> actions;
  bool myBool;
  if (ImGui::Begin("Video controls", &myBool, window_flags)) {

    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, std::clamp(showControls / 60.0f, 0.0f, 1.0f));
    ImGui::SetCursorPosX(30);
    ImGui::SetCursorPosY(work_size.y - 70);
    ImGui::BeginGroup();
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
    ImGui::EndGroup();
    if (ImGui::IsItemHovered()) {
      showControls = 70;
    } else if (showControls > 0) {
      showControls--;
    }
    ImGui::PopStyleVar();


    if (displayState.displayTime) {
      std::string timeStr = vivictpp::time::formatTime(displayState.pts);
      ImVec2 textSize = ImGui::CalcTextSize(timeStr.c_str());
      int pad = 2;
      float y0 = 10;
      float x = (work_size.x - textSize.x) / 2 - pad;
      ImGui::GetWindowDrawList()->AddRectFilled( {x, y0 - pad},
                                                 {x + 2 * pad + textSize.x, y0 + textSize.y + pad},
                                                 transparentBg);
      x = (work_size.x - textSize.x) / 2;
      ImGui::SetCursorPosX(x);
      ImGui::SetCursorPosY(y0);
      ImGui::Text("%s",   timeStr.c_str());
    }

    if (displayState.displayMetadata) {
      ImGui::SetCursorPosX(10.0f);
      ImGui::SetCursorPosY(10.0f);
      leftMetadata.draw(displayState);
      ImGui::SetCursorPosX(work_size.x - 250.0f - 20.0f);
      ImGui::SetCursorPosY(10.0f);
      rightMetadata.draw(displayState);
    }

    ImGui::SetCursorPosX(0);
    ImGui::SetCursorPosY(0);
    if (ImGui::InvisibleButton("##playpause", work_size)) {
      if (wasDragging) {
        wasDragging = false;
      } else {
        actions.push_back({PlayPause});
      }
    };
    if (ImGui::IsMouseDragging(0)) {
      ImVec2 mouseDelta = ImGui::GetIO().MouseDelta;
      actions.push_back({Scroll, 0, mouseDelta});
      wasDragging = true;
    }
  }
  ImGui::End();
  return actions;
}
