// SPDX-FileCopyrightText: 2023 Gustav Grusell (gustav.grusell@gmail.com)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "imgui/WidgetUtils.hh"
#include "imgui.h"

void vivictpp::imgui::comboBox(
    std::string label, const std::vector<std::string> &items,
    std::string &currentItem,
    const std::function<bool(const std::string &)> &exclude) {
  if ((exclude && exclude(currentItem)) ||
      std::find(items.begin(), items.end(), currentItem) == items.end()) {
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

template <>
void vivictpp::imgui::tableRow<std::string>(const char *label,
                                            const char *format,
                                            std::string value) {
  ImGui::TableNextRow();
  ImGui::TableNextColumn();
  ImGui::Text("%s", label);
  ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, transparentBg);
  ImGui::TableNextColumn();
  ImGui::Text(format, value.c_str());
  ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, transparentBg);
}

template <>
void vivictpp::imgui::tableRow2<std::string>(const float firstColumnWidth,
                                             const char *label,
                                             const char *format,
                                             std::string value) {
  tableRow2(firstColumnWidth, label, format, value.c_str());
}
