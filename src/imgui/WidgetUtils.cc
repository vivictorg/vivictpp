// SPDX-FileCopyrightText: 2023 Gustav Grusell (gustav.grusell@gmail.com)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "imgui/WidgetUtils.hh"
#include "imgui.h"

void vivictpp::imgui::comboBox(std::string label,
                               const std::vector<std::string> &items,
                               std::string &currentItem,
                               const std::function<bool(const std::string&)> &exclude) {
  if ((exclude && exclude(currentItem)) || std::find(items.begin(), items.end(), currentItem) == items.end()) {
    currentItem = "";
    for (auto &item : items) {
      if (!exclude || !exclude(item)) {
        currentItem = item;
        break;
      }
    }
  }
  if (ImGui::BeginCombo(label.c_str(), currentItem.c_str(), 0)) {
    for (size_t n = 0; n < items.size(); n++) {
      if (exclude && exclude(items[n])) {
        continue;
      }
      if (ImGui::Selectable(items[n].c_str(), items[n] == currentItem)) {
        currentItem = items[n];
      }

      if (items[n] == currentItem) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }
}
