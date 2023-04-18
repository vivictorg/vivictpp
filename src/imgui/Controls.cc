// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "imgui/Controls.hh"
#include "imgui/Colors.hh"
#include "imgui/Fonts.hh"
#include "imgui.h"
#include "spdlog/logger.h"

void button(const char* text, const char* tooltip, std::function<void(void)> onClick) {
  ImGui::PushFont(vivictpp::imgui::getIconFont());
   if (ImGui::Button(text)) {
     onClick();
   }
   ImGui::PopFont();
   if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
     ImGui::SetTooltip("%s", tooltip);
   }
}

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

  std::vector<Action> actions;
  bool myBool;
  ImVec2 pos = {30, work_size.y - 100};

  ImGui::SetNextWindowPos(work_pos, ImGuiCond_Always);
  ImGui::SetNextWindowSize({work_size.x, work_size.y});
  window_flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground;
  ImGui::SetNextWindowBgAlpha(0.0f); // Transparent background

  if (ImGui::Begin("Video controls", &myBool, window_flags)) {

    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, std::clamp(showControls / 60.0f, 0.0f, 1.0f));
    ImGui::SetCursorPosX(pos.x);
    ImGui::SetCursorPosY(pos.y);
    ImGui::BeginGroup();
    ImGui::PushStyleColor(ImGuiCol_Button, vivictpp::imgui::transparent);


    button(playbackState.playing ? ICON_VPP_PAUSE : ICON_VPP_PLAY,
           playbackState.playing ? "Pause playback" : "Start playback",
           [&actions]{actions.push_back({ActionType::PlayPause});});


    ImGui::SameLine();
    button(ICON_VPP_STEP_BACKWARD, "Step back one frame",
           [&actions]{ actions.push_back({ActionType::StepBackward}); });

    ImGui::SameLine();
    button(ICON_VPP_STEP_FORWARD, "Step forward one frame",
           [&actions]{ actions.push_back({ActionType::StepForward}); });

    ImGui::SameLine();
    button(ICON_VPP_ZOOM_IN,  "Zoom in",
           [&actions]{ actions.push_back({ActionType::ZoomIn}); });

    ImGui::SameLine();
    button(ICON_VPP_ZOOM_OUT, "Zoom out",
           [&actions]{ actions.push_back({ActionType::ZoomOut}); });

    ImGui::SameLine();
    button(ICON_VPP_ZOOM_RESET, "Reset zoom",
           [&actions]{ actions.push_back({ActionType::ZoomReset}); });

    ImGui::SameLine();
    button(displayState.fullscreen ? ICON_VPP_COLLAPSE : ICON_VPP_EXPAND,
           displayState.fullscreen ? "Enter fullscreen mode" : "Exit fullscreen mode",
           [&actions]{ actions.push_back({ActionType::ToggleFullscreen}); });

    ImGui::PopStyleColor();
    float sliderWidth = work_size.x - 60;
    float grabSize = 8.0f;
    float grabPadding = 2.0f; // Copy of value grap_padding in imgui_widgets.cpp
    ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, grabSize);
    ImGui::PushItemWidth(sliderWidth);
    float durationSeconds = playbackState.duration / 1e6;
    ImGui::SliderFloat("##Seekbar", &seekValue, 0.0f,  durationSeconds, "");
    float oldSeekValue = seekValue;
    ImGui::PopItemWidth();
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
      float posFrac = std::clamp((ImGui::GetIO().MousePos.x - pos.x - grabPadding - grabSize / 2) / (sliderWidth - grabSize - 2 * grabPadding), 0.0f, 1.0f);
      ImGui::SetTooltip("%s", vivictpp::time::formatTime(durationSeconds * posFrac, false).c_str());
    }
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
      ImGui::GetWindowDrawList()->AddRectFilled( {x, ImGui::GetWindowPos().y + y0 - pad},
                                                 {x + 2 * pad + textSize.x, ImGui::GetWindowPos().y + y0 + textSize.y + pad},
                                                 transparentBg);
      x = (work_size.x - textSize.x) / 2;
      ImGui::SetCursorPosX(x);
      ImGui::SetCursorPosY(y0);
      ImGui::Text("%s",   timeStr.c_str());
    }

    if (playbackState.speedAdjust != 0) {
      float speed = 1 * std::pow(2, playbackState.speedAdjust * 0.5);
      ImVec2 textSize = ImGui::CalcTextSize("Speed: x0.00");
      int pad = 2;
      float y0 = displayState.displayTime ? 30 : 10;
      float x = (work_size.x - textSize.x) / 2 - pad;
      float rectangleY0 = ImGui::GetWindowPos().y + y0 - pad;
      float rectangleY1 = rectangleY0 + textSize.y + 2 * pad;
      ImGui::GetWindowDrawList()->AddRectFilled( {x, rectangleY0},
                                                 {x + 2 * pad + textSize.x, rectangleY1},
                                                 transparentBg);
      x = (work_size.x - textSize.x) / 2;
      ImGui::SetCursorPosX(x);
      ImGui::SetCursorPosY(y0);
      ImGui::Text("Speed: x%.2f", speed);
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
