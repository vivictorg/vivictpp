// SPDX-FileCopyrightText: 2024 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "imgui/PlotWindow.hh"
#include "libs/implot/implot.h"

const char* PLOT_TYPE_NONE = "None";
const char* PLOT_TYPE_BITRATEGOP = "Bitrate (GOP)";
const char* PLOT_TYPE_FRAMESIZE = "Framesize";

void plotLine(const std::string &type, const std::string name, const vivictpp::video::PlotDatas &plotDatas) {
  const vivictpp::video::PlotData *plotData = nullptr;
  if (type == PLOT_TYPE_BITRATEGOP) {
    plotData = &plotDatas.gopBitrate;
  } else if (type == PLOT_TYPE_FRAMESIZE) {
    plotData = &plotDatas.frameSize;
  }
  if (plotData) {
    ImPlot::PlotLine(name.c_str(), (float*)plotData->pts.data(), plotData->values.data(), plotData->pts.size());
  }
}

std::string plotLineName(const std::string &type, const std::string &source) {
  return type + " - " + std::filesystem::path(source).filename().string();
}

 int formatTime(double value, char* buff, int size, void* userData) {
    bool *includeMs = (bool*)userData;
    int hours = int(value / 3600);
    int minutes = int((value - hours * 3600) / 60);
    int seconds = int(value - hours * 3600 - minutes * 60);
    int milliseconds = int((value - hours * 3600 - minutes * 60 - seconds) * 1000);
    if (*includeMs) {
      snprintf(buff, size, "%02d:%02d:%02d.%03d", hours, minutes, seconds, milliseconds);
    } else {
      snprintf(buff, size, "%02d:%02d:%02d", hours, minutes, seconds);
    }
    return 0;
 }

 int formatBitrate(double value, char* buff, int size, void* userData) {
  bool *isBitrate = (bool*)userData;
  if (*isBitrate) {
    if (value > 1e6) {
      snprintf(buff, size, "%.2f Mbit/s", value / 1e6);
    } else {
      snprintf(buff, size, "%.2f kbit/s", value / 1000);
    }
  } else {
    snprintf(buff, size, "%d bytes", int(value));
  }
  return 0;
 }

std::vector<vivictpp::imgui::Action>
vivictpp::imgui::PlotWindow::draw(const ui::DisplayState &displayState) {
  std::vector<vivictpp::imgui::Action> actions;
  if (!displayState.displayPlot) {
    return actions;
  }

  static bool transparentBg = false;
  ImGui::Begin("Plot window", nullptr,
               transparentBg ? ImGuiWindowFlags_NoBackground
                             : ImGuiWindowFlags_None);

  ImGui::Text("Transparent Background");
  ImGui::SameLine();
  ImGui::Checkbox("##Transparent background", &transparentBg);
  static double timeX[2] = {0.0, 0.0};
  static double timeY[2] = {0.0, 0.0};
  timeX[0] = timeX[1] = vivictpp::time::ptsToDouble(displayState.pts);
  static std::string plotType = PLOT_TYPE_BITRATEGOP;
  int resetTimePlotY = 0;
  std::string previousPlotType = plotType;
  ImGui::SameLine();
  ImGui::Text("Left plot");
  ImGui::SameLine();
  ImGui::SetNextItemWidth(150.);
  if (ImGui::BeginCombo("##Plot type", plotType.c_str())) {
    if (ImGui::Selectable("Bitrate (GOP)", plotType == PLOT_TYPE_BITRATEGOP)) {
      plotType = PLOT_TYPE_BITRATEGOP;
    }
    if (ImGui::Selectable("Frame size", plotType == PLOT_TYPE_FRAMESIZE)) {
      plotType = PLOT_TYPE_FRAMESIZE;
    }
    ImGui::EndCombo();
  }
  resetTimePlotY = previousPlotType != plotType ? 2 : 0;
  // timeY[0] = 0.0;
  // timeY[1] = 2000;
  ImPlot::PushStyleColor(ImPlotCol_FrameBg,
                         {0, 0, 0, transparentBg ? 0.4f : 1.0f});
  ImPlot::PushStyleColor(ImPlotCol_PlotBg, {0, 0, 0, 0});
  if (ImPlot::BeginPlot("## Plot", ImVec2(-1, -1), ImPlotFlags_NoMenus)) {
    // ImPlot::GetInputMap().Fit = ImGuiMouseButton_Right;
    // ImPlot::GetInputMap().Menu = ImGuiMouseButton_Middle;
    static ImPlotRect rect;
    ImPlot::SetupAxis(ImAxis_X1, nullptr, ImPlotAxisFlags_NoMenus);
    ImPlot::SetupAxis(ImAxis_Y1, nullptr,
                      ImPlotAxisFlags_NoMenus | ImPlotAxisFlags_AutoFit);
    bool includeMs = rect.X.Max - rect.X.Min < 10;
    ImPlot::SetupAxisFormat(ImAxis_X1, formatTime, &includeMs);
    bool isBitrate = plotType == PLOT_TYPE_BITRATEGOP;
    ImPlot::SetupAxisFormat(ImAxis_Y1, formatBitrate, &isBitrate);
    plotLine(plotType, plotLineName(plotType, displayState.leftVideoMetadata.source), leftVideoIndex->getPlotDatas());
    bool hasRightSource = rightVideoIndex->ready();
    if (hasRightSource) {
      plotLine(plotType, plotLineName(plotType, displayState.rightVideoMetadata.source), rightVideoIndex->getPlotDatas());
    }
    if (resetTimePlotY == 2) {
      timeY[1] = 0;
    }
    ImPlot::PlotLine("Time", timeX, timeY, 2);
    rect = ImPlot::GetPlotLimits();
    timeY[0] = rect.Y.Min;
    if (resetTimePlotY > 0) {
      resetTimePlotY--;
    }
    if (resetTimePlotY == 0) {
      timeY[1] = rect.Y.Max;
    }
    //timeY[1] = rect.Y.Max;
    bool isHovered = ImGui::IsItemHovered();
    bool isFocused = ImGui::IsItemFocused();
    if (isHovered && isFocused &&
        ImGui::IsMouseClicked(ImGuiMouseButton_Left) &&
        ImGui::GetIO().KeyCtrl) {

      ImPlotPoint ipp = ImPlot::GetPlotMousePos();
      spdlog::warn("Mouse clicked in plot window {}, {}", ipp.x, ipp.y);
      actions.push_back({ActionType::Seek, vivictpp::time::doubleToPts(ipp.x)});
    }
    ImPlot::EndPlot();
  }
  ImPlot::PopStyleColor(2);
  ImGui::End();

  return actions;
}

