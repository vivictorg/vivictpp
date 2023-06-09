// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "imgui/VideoMetadataDisplay.hh"
#include "imgui.h"
#include "time/TimeUtils.hh"
#include "imgui/Colors.hh"
#include "ui/FontSize.hh"
#include "imgui/WidgetUtils.hh"

const float FIRST_COLUMN_WIDTH = 150.0f;
const float SECOND_COLUMN_WIDTH = 100.0f;

float vivictpp::imgui::VideoMetadataDisplay::calcWidth() { return vivictpp::ui::FontSize::getScaleFactor() * (FIRST_COLUMN_WIDTH + SECOND_COLUMN_WIDTH); }


void vivictpp::imgui::VideoMetadataDisplay::draw(const ui::DisplayState &displayState) {
  const VideoMetadata &metadata = (type == Type::LEFT) ? displayState.leftVideoMetadata :
                                  displayState.rightVideoMetadata;
  if (metadata.empty()) return;
  float scaling = vivictpp::ui::FontSize::getScaleFactor();
  ImGui::BeginGroup();
  ImGui::BeginTable("metadata", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_NoHostExtendX);
  ImGui::TableSetupColumn("1",ImGuiTableColumnFlags_WidthFixed, FIRST_COLUMN_WIDTH * scaling);
  ImGui::TableSetupColumn("2",ImGuiTableColumnFlags_WidthFixed, SECOND_COLUMN_WIDTH * scaling);
  vivictpp::imgui::tableRow("codec", "%s", metadata.codec);
  vivictpp::imgui::tableRow("resolution", "%s", metadata.filteredResolution.toString());
  if (metadata.filteredResolution != metadata.resolution) {
    vivictpp::imgui::tableRow("orig resolution", "%s", metadata.resolution.toString());
  }
  vivictpp::imgui::tableRow("bitrate", "%dkb/s", metadata.bitrate / 1000);
  vivictpp::imgui::tableRow("framerate", "%.3ffps", metadata.frameRate);
  vivictpp::imgui::tableRow("duration", "%s", vivictpp::time::formatTime(metadata.duration));
  vivictpp::imgui::tableRow("start time", "%s", vivictpp::time::formatTime(metadata.startTime));
//  metadataText.push_back({"pixel format", metadata.pixelFormat});

  ImGui::EndTable();
  if (!displayState.isPlaying) {
    const FrameMetadata &frameMetadata = (type == Type::LEFT) ? displayState.leftFrame.metadata() :
                                  displayState.rightFrame.metadata();
    ImGui::BeginTable("framemetadata", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_NoHostExtendX);
    ImGui::TableSetupColumn("1",ImGuiTableColumnFlags_WidthFixed, FIRST_COLUMN_WIDTH * scaling);
    ImGui::TableSetupColumn("2",ImGuiTableColumnFlags_WidthFixed, SECOND_COLUMN_WIDTH * scaling);

    vivictpp::imgui::tableRow("Frame type", "%c", frameMetadata.pictureType);
    vivictpp::imgui::tableRow("Frame size", "%d", frameMetadata.size);
    ImGui::EndTable();
  }
  ImGui::EndGroup();
}
