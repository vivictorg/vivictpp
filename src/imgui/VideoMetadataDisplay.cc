// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "imgui/VideoMetadataDisplay.hh"
#include "time/TimeUtils.hh"
#include "imgui/Colors.hh"
#include "ui/FontSize.hh"


void vivictpp::imgui::VideoMetadataDisplay::initMetadataText(const ui::DisplayState &displayState) {
  const VideoMetadata &metadata = (type == Type::LEFT) ? displayState.leftVideoMetadata :
                                  displayState.rightVideoMetadata;
  metadataText.clear();
  if (metadata.empty()) return;
  metadataText.push_back({"codec", metadata.codec});
  metadataText.push_back({"decoder", metadata.decoderName});
  metadataText.push_back({"hardware accelerated", metadata.hwAccel ? "true" : "false"});
  metadataText.push_back({"resolution", metadata.filteredResolution.toString()});
  if (metadata.filteredResolution != metadata.resolution) {
      metadataText.push_back({"orig resolution", metadata.resolution.toString()});
  }
  metadataText.push_back({"bitrate", std::to_string(metadata.bitrate / 1000) + "kb/s"});
  metadataText.push_back({"framerate", std::to_string(metadata.frameRate) + "fps"});
  metadataText.push_back({"duration", vivictpp::time::formatTime(metadata.duration)});
  metadataText.push_back({"start time", vivictpp::time::formatTime(metadata.startTime)});
  metadataText.push_back({"pixel format", metadata.pixelFormat});
}

void vivictpp::imgui::VideoMetadataDisplay::initFrameMetadataText(const ui::DisplayState &displayState) {
  const FrameMetadata &metadata = (type == Type::LEFT) ? displayState.leftFrame.metadata() :
                                  displayState.rightFrame.metadata();
  frameMetadataText.clear();
  frameMetadataText.push_back({"Frame type", std::string(1,metadata.pictureType)});
  frameMetadataText.push_back({"Frame size", std::to_string(metadata.size)});
}

void vivictpp::imgui::VideoMetadataDisplay::draw(const ui::DisplayState &displayState) {
  if (displayState.videoMetadataVersion != metadataVersion) {
    initMetadataText(displayState);
  }
  if (metadataText.empty()) return;
  float scaling = vivictpp::ui::FontSize::getScaleFactor();
  ImGui::BeginGroup();
  ImGui::BeginTable("metadata", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_NoHostExtendX);
  ImGui::TableSetupColumn("1",ImGuiTableColumnFlags_WidthFixed, 150.0f * scaling);
  ImGui::TableSetupColumn("2",ImGuiTableColumnFlags_WidthFixed, 100.0f * scaling);
  for (const auto &data : metadataText) {
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::Text("%s", data.first.c_str());
    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, transparentBg);
    ImGui::TableNextColumn();
    ImGui::Text("%s", data.second.c_str());
    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, transparentBg);
  }
  ImGui::EndTable();
  if (!displayState.isPlaying) {
    initFrameMetadataText(displayState);
    ImGui::BeginTable("framemetadata", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_NoHostExtendX);
    ImGui::TableSetupColumn("1",ImGuiTableColumnFlags_WidthFixed, 150.0f * scaling);
    ImGui::TableSetupColumn("2",ImGuiTableColumnFlags_WidthFixed, 100.0f * scaling);
    for (const auto &data : frameMetadataText) {
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("%s", data.first.c_str());
      ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, transparentBg);
      ImGui::TableNextColumn();
      ImGui::Text("%s", data.second.c_str());
      ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, transparentBg);
    }
    ImGui::EndTable();
  }
  ImGui::EndGroup();
}
