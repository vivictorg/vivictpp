// SPDX-FileCopyrightText: 2024 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "imgui/PlotWindow.hh"
#include "libs/implot/implot.h"

const char *PLOT_TYPE_NONE = "None";
const char *PLOT_TYPE_BITRATEGOP = "Bitrate (GOP)";
const char *PLOT_TYPE_FRAMESIZE = "Framesize";

void plotLine(const std::string &type, const std::string name,
              const vivictpp::video::PlotDatas &plotDatas,
              const std::shared_ptr<vivictpp::qualitymetrics::QualityMetrics>
                  qualityMetrics) {
  const vivictpp::video::PlotData *plotData = nullptr;
  if (type == PLOT_TYPE_BITRATEGOP || type == PLOT_TYPE_FRAMESIZE) {
    plotData = type == PLOT_TYPE_BITRATEGOP ? &plotDatas.gopBitrate
                                            : &plotDatas.frameSize;
    ImPlot::PlotLine(name.c_str(), (float *)plotData->pts.data(),
                     plotData->values.data(), plotData->pts.size());
  } else {
    if (!qualityMetrics || !qualityMetrics->hasMetric(type)) {
      return;
    }
    const auto &metric = qualityMetrics->getMetric(type);
    float *ptsData = (float *)plotDatas.frameSize.pts.data();
    ImPlot::PlotLine(name.c_str(), ptsData, (float *)metric.data(),
                     metric.size());
  }
}

std::string plotLineName(const std::string &type, const std::string &source) {
  return type + " - " + std::filesystem::path(source).filename().string();
}

int formatTime(double value, char *buff, int size, void *userData) {
  bool *includeMs = (bool *)userData;
  int hours = int(value / 3600);
  int minutes = int((value - hours * 3600) / 60);
  int seconds = int(value - hours * 3600 - minutes * 60);
  int milliseconds =
      int((value - hours * 3600 - minutes * 60 - seconds) * 1000);
  if (*includeMs) {
    snprintf(buff, size, "%02d:%02d:%02d.%03d", hours, minutes, seconds,
             milliseconds);
  } else {
    snprintf(buff, size, "%02d:%02d:%02d", hours, minutes, seconds);
  }
  return 0;
}

int formatBitrate(double value, char *buff, int size, void *userData) {
  std::string plotType = *(std::string *)userData;
  if (plotType == PLOT_TYPE_BITRATEGOP) {
    if (value > 1e6) {
      snprintf(buff, size, "%.2f Mbit/s", value / 1e6);
    } else {
      snprintf(buff, size, "%.2f kbit/s", value / 1000);
    }
  } else if (plotType == PLOT_TYPE_FRAMESIZE) {
    snprintf(buff, size, "%d bytes", int(value));
  } else {
    snprintf(buff, size, "%.2f", value);
  }
  return 0;
}

std::vector<std::pair<std::string, std::string>>
vivictpp::imgui::PlotWindow::getSelectableQualityMetrics(
    const vivictpp::ui::DisplayState &displayState) {
  std::vector<std::pair<std::string, std::string>> metrics;
  if (displayState.leftQualityMetrics) {
    for (const auto &metric : displayState.leftQualityMetrics->getMetrics()) {
      std::string name =
          displayState.rightQualityMetrics &&
                  displayState.rightQualityMetrics->hasMetric(metric)
              ? metric
              : metric + " (left only)";
      metrics.push_back({name, metric});
    }
  }
  if (displayState.rightQualityMetrics) {
    for (const auto &metric : displayState.rightQualityMetrics->getMetrics()) {
      if (displayState.leftQualityMetrics &&
          displayState.leftQualityMetrics->hasMetric(metric)) {
        continue;
      }
      metrics.push_back({metric + " (right only)", metric});
    }
  }
  return metrics;
}

std::vector<vivictpp::imgui::Action>
vivictpp::imgui::PlotWindow::draw(const ui::DisplayState &displayState) {
  std::vector<vivictpp::imgui::Action> actions;
  if (!displayState.displayPlot) {
    return actions;
  }

  static bool transparentBg = false;
  static bool showPlotWindow = true;
  static bool autofitX = true;
  static bool autofitY = true;
  showPlotWindow = displayState.displayPlot;
  if (ImGui::Begin("Plot window", &showPlotWindow,
                   transparentBg ? ImGuiWindowFlags_NoBackground
                                 : ImGuiWindowFlags_None)) {

    ImGui::Text("Transparent Background");
    ImGui::SameLine();
    ImGui::Checkbox("##Transparent background", &transparentBg);
    static std::string plotType = PLOT_TYPE_BITRATEGOP;

    std::string previousPlotType = plotType;
    ImGui::SameLine();
    ImGui::Text("Left plot");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(150.);
    if (ImGui::BeginCombo("##Plot type", plotType.c_str())) {
      if (ImGui::Selectable("Bitrate (GOP)",
                            plotType == PLOT_TYPE_BITRATEGOP)) {
        plotType = PLOT_TYPE_BITRATEGOP;
      }
      if (ImGui::Selectable("Frame size", plotType == PLOT_TYPE_FRAMESIZE)) {
        plotType = PLOT_TYPE_FRAMESIZE;
      }
      auto selectableMetrics = getSelectableQualityMetrics(displayState);
      for (const auto &metric : selectableMetrics) {
        if (ImGui::Selectable(metric.first.c_str(),
                              plotType == metric.second)) {
          plotType = metric.second;
        }
      }
      ImGui::EndCombo();
    }
    ImGui::SameLine();
    ImGui::Text("Autofix X");
    ImGui::SameLine();
    ImGui::Checkbox("##Autofit X", &autofitX);
    ImGui::SameLine();
    ImGui::Text("Autofix Y");
    ImGui::SameLine();
    ImGui::Checkbox("##Autofit Y", &autofitY);

    ImPlot::PushStyleColor(ImPlotCol_FrameBg,
                           {0, 0, 0, transparentBg ? 0.4f : 1.0f});
    ImPlot::PushStyleColor(ImPlotCol_PlotBg, {0, 0, 0, 0});
    if (ImPlot::BeginPlot("## Plot", ImVec2(-1, -1), ImPlotFlags_NoMenus)) {
      // ImPlot::GetInputMap().Fit = ImGuiMouseButton_Right;
      // ImPlot::GetInputMap().Menu = ImGuiMouseButton_Middle;
      static ImPlotRect rect;
      ImPlot::SetupAxis(ImAxis_X1, nullptr,
                        ImPlotAxisFlags_NoMenus |
                            (autofitX ? ImPlotAxisFlags_AutoFit : 0));
      ImPlot::SetupAxis(ImAxis_Y1, nullptr,
                        ImPlotAxisFlags_NoMenus |
                            (autofitY ? ImPlotAxisFlags_AutoFit : 0));
      bool includeMs = rect.X.Max - rect.X.Min < 10;
      ImPlot::SetupAxisFormat(ImAxis_X1, formatTime, &includeMs);
      // bool isBitrate = plotType == PLOT_TYPE_BITRATEGOP;
      ImPlot::SetupAxisFormat(ImAxis_Y1, formatBitrate, &plotType);
      plotLine(plotType,
               plotLineName(plotType, displayState.leftVideoMetadata.source),
               leftVideoIndex->getPlotDatas(), displayState.leftQualityMetrics);
      bool hasRightSource = rightVideoIndex->ready();
      if (hasRightSource) {
        plotLine(plotType,
                 plotLineName(plotType, displayState.rightVideoMetadata.source),
                 rightVideoIndex->getPlotDatas(),
                 displayState.rightQualityMetrics);
      }

      bool isHovered = ImGui::IsItemHovered();
      bool isFocused = ImGui::IsItemFocused();
      if (isHovered && isFocused &&
          ImGui::IsMouseClicked(ImGuiMouseButton_Left) &&
          ImGui::GetIO().KeyCtrl) {

        ImPlotPoint ipp = ImPlot::GetPlotMousePos();
        spdlog::warn("Mouse clicked in plot window {}, {}", ipp.x, ipp.y);
        actions.push_back(
            {ActionType::Seek, vivictpp::time::doubleToPts(ipp.x)});
      }
      drawPtsMarker(displayState);
      ImPlot::EndPlot();
    }
    ImPlot::PopStyleColor(2);
  }
  ImGui::End();
  if (!showPlotWindow) {
    actions.push_back({ActionType::ToggleDisplayPlot});
  }

  return actions;
}

void vivictpp::imgui::PlotWindow::drawPtsMarker(
    const ui::DisplayState &displayState) {
  auto plotPos = ImPlot::GetPlotPos();
  auto plotSize = ImPlot::GetPlotSize();
  auto timePoint = ImPlot::PlotToPixels(
      vivictpp::time::ptsToDouble(displayState.pts), 0, ImAxis_X1, ImAxis_Y1);
  auto p1 = ImVec2(timePoint.x, plotPos.y);
  auto p2 = ImVec2(timePoint.x - 4, plotPos.y - 7);
  auto p3 = ImVec2(timePoint.x + 4, plotPos.y - 7);
  ImGui::GetWindowDrawList()->AddLine(
      ImVec2(timePoint.x, plotPos.y),
      ImVec2(timePoint.x, plotPos.y + plotSize.y), IM_COL32(255, 255, 255, 255),
      0.5f);
  ImGui::GetWindowDrawList()->AddTriangleFilled(p1, p2, p3,
                                                IM_COL32(255, 255, 255, 255));
}