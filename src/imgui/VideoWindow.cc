// SPDX-FileCopyrightText: 2024 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "imgui/VideoWindow.hh"
#include "ImGuiFileDialog.h"
#include "SDL_video.h"
#include "Settings.hh"
#include "SourceConfig.hh"
#include "VivictPPConfig.hh"
#include "imgui.h"
#include "imgui/About.hh"
#include "imgui/Colors.hh"
#include "imgui/Events.hh"
#include "imgui/Fonts.hh"
#include "imgui/Help.hh"
#include "imgui/Logs.hh"
#include "imgui/Splash.hh"
#include "imgui/VivictPPImGui.hh"
#include "imgui_internal.h"
#include "sdl/SDLUtils.hh"
#include "spdlog/logger.h"
#include "time/TimeUtils.hh"
#include "ui/DisplayState.hh"
#include <map>
#include <memory>

float alignForWidthPos(float width, float alignment = 0.5f,
                       float offset = 0.0f) {
  //   const ImGuiViewport* viewport = ImGui::GetMainViewport();
  // ImVec2 work_size = viewport->WorkSize;
  float avail = ImGui::GetContentRegionAvail().x;
  float off = (avail - width) * alignment;
  return ImGui::GetCursorPosX() + std::max(0.0f, off) + offset;
}

void alignForWidth(float width, float alignment = 0.5f, float offset = 0.0f) {
  ImGui::SetCursorPosX(alignForWidthPos(width, alignment, offset));
}

void VideoWindow::onZoomChange(const Resolution &nativeResolution,
                               const vivictpp::ui::Zoom &zoom) {
  ImVec2 oldVideoSize = videoSize;

  videoSize = {nativeResolution.w * zoom.multiplier(),
               nativeResolution.h * zoom.multiplier()};
  if (videoSize.x > size.x || videoSize.y > size.y) {
    // Adjust scroll so center of picture stays the same
    // TODO: Take 'pad' into account maybe
    ImVec2 center = {(scroll.x + size.x / 2) / oldVideoSize.x,
                     (scroll.y + size.y / 2) / oldVideoSize.y};
    scroll = {center.x * videoSize.x - size.x / 2,
              center.y * videoSize.y - size.y / 2};
    scrollUpdated = true;
  } else {
    if (scroll.x != 0.0 || scroll.y != 0.0) {
      scroll = {0, 0};
      scrollUpdated = true;
    }
  }
}
void VideoWindow::onScroll(const ImVec2 &scrollDelta) {
  // TODO: Limit scroll
  float maxScrollX = std::max(0.0f, videoSize.x - size.x);
  float maxScrollY = std::max(0.0f, videoSize.y - size.y);
  scroll = {std::clamp(scroll.x - scrollDelta.x, 0.0f, maxScrollX),
            std::clamp(scroll.y - scrollDelta.y, 0.0f, maxScrollY)};
  scrollUpdated = true;
}

ImVec2 resolutionToImVec2(const Resolution &resolution) {
  return {(float)resolution.w, (float)resolution.h};
}

void VideoWindow::draw(vivictpp::ui::VideoTextures &videoTextures,
                       const vivictpp::ui::DisplayState &displayState) {
  const ImGuiViewport *viewport = ImGui::GetMainViewport();
  ImVec2 work_size = viewport->WorkSize;
  ImVec2 work_pos = viewport->WorkPos;
  pos = {0, work_pos.y};
  size = work_size;
  ImGui::SetNextWindowPos(pos);
  ImGui::SetNextWindowSize(size);

  float scaleFactor = displayState.zoom.multiplier();
  if (displayState.fitToScreen) {
    scaleFactor *= std::min(work_size.x / videoTextures.nativeResolution.w,
                            work_size.y / videoTextures.nativeResolution.h);
  }

  ImVec2 scaledVideoSize = {videoTextures.nativeResolution.w * scaleFactor,
                            videoTextures.nativeResolution.h * scaleFactor};
  videoSize = scaledVideoSize;
  ImGui::SetNextWindowContentSize({std::max(work_size.x, scaledVideoSize.x),
                                   std::max(work_size.y, scaledVideoSize.y)});
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

  bool myBool2;
  if (ImGui::Begin(
          "Vivict++", &myBool2,
          ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration |
              ImGuiWindowFlags_NoSavedSettings |
              ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav |
              ImGuiWindowFlags_NoBringToFrontOnFocus |
              ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar)) {
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

    ImVec2 pad; // = {viewport->Size.x - work_size.x, viewport->Size.y -
                // work_pos.y};
    if (scaledVideoSize.x < viewSize.x) {
      pad.x += std::floor((viewSize.x - scaledVideoSize.x) / 2);
    }
    if (scaledVideoSize.y < viewSize.y) {
      pad.y += std::floor((viewSize.y - scaledVideoSize.y) / 2);
    }
    videoPos = pad;

    ImVec2 leftScaledSize = resolutionToImVec2(displayState.leftVideoMetadata.displayResolution.scaleKeepingAspectRatio(scaledVideoSize.x, scaledVideoSize.y));
    ImVec2 leftPad = {(scaledVideoSize.x - leftScaledSize.x)/2, (scaledVideoSize.y - leftScaledSize.y)/2};

    float splitX =
        std::clamp(ImGui::GetMousePos().x, pad.x, pad.x + scaledVideoSize.x);
    ImVec2 drawPos = {pad.x + leftPad.x - scrollX, pad.y + leftPad.y - scrollY}; // cursorPos;
    ImVec2 uvMin(0, 0);

    ImVec2 uvMax(std::clamp((scrollX + splitX - pad.x - leftPad.x) / leftScaledSize.x, 0.0f, 1.0f), 1);
    ImVec2 p2(std::min(splitX, drawPos.x + leftScaledSize.x), drawPos.y + leftScaledSize.y);
    ImGui::GetWindowDrawList()->AddImage(
        (ImTextureID)(intptr_t)videoTextures.leftTexture.get(), drawPos,
        p2 , uvMin, uvMax);

    if (!displayState.splitScreenDisabled) {
      ImVec2 rightScaledSize =  resolutionToImVec2(displayState.rightVideoMetadata.displayResolution.scaleKeepingAspectRatio(scaledVideoSize.x, scaledVideoSize.y));
      ImVec2 rightPad = {(scaledVideoSize.x - rightScaledSize.x)/2, (scaledVideoSize.y - rightScaledSize.y)/2};
      
      p2 = {pad.x + rightPad.x + rightScaledSize.x - scrollX,
              pad.y + rightPad.y + rightScaledSize.y - scrollY};
      drawPos.x = std::clamp(splitX, pad.x + rightPad.x - scrollX, pad.x + rightPad.x - scrollX + rightScaledSize.x);
      drawPos.y = pad.y + rightPad.y - scrollY;
      uvMin.x = std::clamp((scrollX + splitX - pad.x - rightPad.x) / rightScaledSize.x, 0.0f, 1.0f);
      uvMax.x = 1.0; //(scrollX + viewSize.x) / scaledVideoSize.x;
      
      ImGui::GetWindowDrawList()->AddImage(
          (ImTextureID)(intptr_t)videoTextures.rightTexture.get(), drawPos, p2,
          uvMin, uvMax);
      ImGui::GetWindowDrawList()->AddLine({splitX, pad.y},
                                          {splitX, pad.y + scaledVideoSize.y},
                                          0x80FFFFFF, 0.5);
    }
  }
  ImGui::End();
  ImGui::PopStyleVar(2);
}
