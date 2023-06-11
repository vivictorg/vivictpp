// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "imgui/VideoMetadataDisplay.hh"
#include "imgui.h"
#include "libav/DecoderMetadata.hh"
#include "time/TimeUtils.hh"
#include "imgui/Colors.hh"
#include "ui/FontSize.hh"
#include "imgui/WidgetUtils.hh"
#include <filesystem>

const float FIRST_COLUMN_WIDTH = 150.0f;
const float SECOND_COLUMN_WIDTH = 100.0f;
const float PAD = 4.0f;

float vivictpp::imgui::VideoMetadataDisplay::calcWidth() { return vivictpp::ui::FontSize::getScaleFactor() * (FIRST_COLUMN_WIDTH + SECOND_COLUMN_WIDTH); }

void lineHere(float x0, float x1) {
  float y = ImGui::GetCursorPosY() + ImGui::GetTextLineHeight();
  ImGui::GetWindowDrawList()->AddLine({x0, y}, {x1, y}, vivictpp::imgui::border);
}

void vivictpp::imgui::VideoMetadataDisplay::draw(const ui::DisplayState &displayState) {
  const VideoMetadata &metadata = (type == Type::LEFT) ? displayState.leftVideoMetadata :
                                  displayState.rightVideoMetadata;
  if (metadata.empty()) return;
  const libav::DecoderMetadata decoderMetadata = (type == Type::LEFT) ? displayState.leftDecoderMetadata :
                                                 displayState.rightDecoderMetadata;
  float width = calcWidth();
  float scaling = vivictpp::ui::FontSize::getScaleFactor();

  p1 = {ImGui::GetCursorPosX() - PAD, ImGui::GetCursorPosY() + ImGui::GetTextLineHeight()};
  p2.x = ImGui::GetCursorPosX() + width + PAD;
  ImGui::GetWindowDrawList()->AddRectFilled( p1, p2,transparentBg);
  ImGui::GetWindowDrawList()->AddRect( p1, p2, border, 0.5f);

  std::filesystem::path sourcePath(metadata.source);
  ImGui::BeginGroup();
  float cw = FIRST_COLUMN_WIDTH * scaling;
  ImGui::Text("%s", sourcePath.filename().c_str());
  if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
    ImGui::SetTooltip("%s", sourcePath.c_str());
   }
  ImGui::Dummy({2.0f, 1.0f});

  vivictpp::imgui::tableRow2(cw, "codec", "%s", metadata.codec);
  vivictpp::imgui::tableRow2(cw, "pixel format", "%s", metadata.pixelFormat);
  vivictpp::imgui::tableRow2(cw, "resolution", "%s", metadata.filteredResolution.toString());

  if (metadata.filteredResolution != metadata.resolution) {
    vivictpp::imgui::tableRow2(cw, "orig resolution", "%s", metadata.resolution.toString());
  }
  vivictpp::imgui::tableRow2(cw, "bitrate", "%dkb/s", metadata.bitrate / 1000);
  vivictpp::imgui::tableRow2(cw, "framerate", "%.3ffps", metadata.frameRate);
  vivictpp::imgui::tableRow2(cw, "duration", "%s", vivictpp::time::formatTime(metadata.duration));
  vivictpp::imgui::tableRow2(cw, "start time", "%s", vivictpp::time::formatTime(metadata.startTime));
  ImGui::Dummy({2.0f, 1.0f});
  vivictpp::imgui::tableRow2(cw, "decoder", "%s", decoderMetadata.name);
  vivictpp::imgui::tableRow2(cw, "hardware acceleration", "%s", decoderMetadata.hwAccel);
  if (decoderMetadata.hwAccel != "none") {
      vivictpp::imgui::tableRow2(cw, "hardware pix fmt", "%s", decoderMetadata.hwPixelFormat);
  }
  if (!displayState.isPlaying) {
    ImGui::Dummy({2.0f, 1.0f});
    const FrameMetadata &frameMetadata = (type == Type::LEFT) ? displayState.leftFrame.metadata() :
                                         displayState.rightFrame.metadata();
    vivictpp::imgui::tableRow2(cw, "Frame type", "%c", frameMetadata.pictureType);
    vivictpp::imgui::tableRow2(cw, "Frame size", "%d", frameMetadata.size);
  }
  p2.y = ImGui::GetCursorPosY() + ImGui::GetTextLineHeight() + PAD;

  ImGui::EndGroup();
}
