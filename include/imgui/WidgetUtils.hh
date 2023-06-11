// SPDX-FileCopyrightText: 2023 Gustav Grusell (gustav.grusell@gmail.com)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIVICTPP_IMGUI_WIDGETUTILS_HH_
#define VIVICTPP_IMGUI_WIDGETUTILS_HH_

#include <string>
#include <vector>
#include <functional>
#include "imgui.h"
#include "logging/Logging.hh"
#include "imgui/Colors.hh"

namespace vivictpp::imgui {

void comboBox(std::string label, const std::vector<std::string> &items, std::string &currentItem, const std::function<bool(const std::string&)> &exclude = {});

template <typename T>
void prioritizedList (const char* str_id,
                      const float &width,
                      std::vector<T> &items,
                      const std::function<void(T&)> &render, bool enableRemove = false,
                      const std::function<void(T&)> &onRemove = {}){
  ImGui::PushID(str_id);
  for (size_t n=0; n < items.size(); n++) {
    ImGui::PushItemWidth(8.0f);
    ImGui::PushID(n);
    render(items[n]);
    ImGui::SameLine(width);
    if (n == 0) {
      ImGui::BeginDisabled();
    }
    if (ImGui::ArrowButton("##moveup", ImGuiDir_Up)) {
      T current = items[n];
      int n_next = n-1;
      items[n] = items[n_next];
      items[n_next] = current;
    }
    if (n == 0) {
      ImGui::EndDisabled();
    }
    ImGui::SameLine();
    if (n == items.size() - 1) {
      ImGui::BeginDisabled();
    }
    if (ImGui::ArrowButton("##movedown", ImGuiDir_Down)) {
      T current = items[n];
      int n_next = n+1;
      items[n] = items[n_next];
      items[n_next] = current;
    }
    if (n == items.size() - 1) {
      ImGui::EndDisabled();
    }
    if (enableRemove) {
      ImGui::SameLine();
      if (ImGui::SmallButton("x")) {
        auto removed = items[n];
        items.erase(std::next(items.begin(), n));
        if (onRemove) {
          onRemove(removed);
        }
      }
    }
    ImGui::PopID();
    ImGui::PopItemWidth();
  }

  ImGui::PopID();
}

template <typename T>
void tableRow(const char* label, const char* format, T value) {
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::Text("%s", label);
    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, transparentBg);
    ImGui::TableNextColumn();
    ImGui::Text(format, value);
    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, transparentBg);
}

template <typename T>
void tableRow2(float firstColumnWidth, const char* label, const char* format, T value) {
    ImGui::Text("%s", label);
    ImGui::SameLine(firstColumnWidth);
    ImGui::Text(format, value);
}

}  // namespace vivictpp::imgui


#endif /* VIVICTPP_IMGUI_WIDGETUTILS_HH_ */
