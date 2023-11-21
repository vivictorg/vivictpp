// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "imgui/MainMenu.hh"
#include "imgui.h"
#include "imgui/Events.hh"

std::vector<vivictpp::imgui::Action> vivictpp::imgui::MainMenu::draw(
  const PlaybackState &playbackState,
  const ui::DisplayState &displayState) {
  std::vector<Action> actions;
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      ImGui::MenuItem("(File menu)", NULL, false, false);
      if (ImGui::MenuItem("Open left", "Ctrl+O")) {
        actions.push_back({ActionType::ShowFileDialogLeft});
      }
      if (ImGui::MenuItem("Open right", "Ctrl+Alt+O", false, playbackState.hasLeftSource)) {
        actions.push_back({ActionType::ShowFileDialogRight});
      }
      ImGui::Separator();
      if (ImGui::MenuItem("Settings", "Ctrl+Alt+S")) {
        actions.push_back({ActionType::ShowSettingsDialog});
      }
      ImGui::Separator();
      if (ImGui::MenuItem("Quit", "Ctrl+Q, Q")) {
        actions.push_back({ActionType::ActionQuit});
      }
      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("View")) {
      ImGui::MenuItem("(View menu)", NULL, false, false);
      if (ImGui::MenuItem("Zoom In", "U")) {
        actions.push_back({ActionType::ZoomIn});
      }
      if (ImGui::MenuItem("Zoom Out", "I")) {
        actions.push_back({ActionType::ZoomOut});
      }
      if (ImGui::MenuItem("Reset Zoom", "0")) {
        actions.push_back({ActionType::ZoomReset});
      }
      if (ImGui::MenuItem("Fullscreen", "F", displayState.fullscreen)) {
        actions.push_back({ActionType::ToggleFullscreen});
      }
      if (ImGui::MenuItem("Display Time", "T", displayState.displayTime)) {
        actions.push_back({ActionType::ToggleDisplayTime});
      }
      if (ImGui::MenuItem("Display Metadata", "D", displayState.displayMetadata)) {
        actions.push_back({ActionType::ToggleDisplayMetadata});
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Playback")) {
      ImGui::MenuItem("(Playback menu)", NULL, false, false);
      if (ImGui::MenuItem("Play", "Space", false, !playbackState.playing)) {
        actions.push_back({ActionType::PlayPause});
      }
      if (ImGui::MenuItem("Pause", "Space", false, playbackState.playing)) {
        actions.push_back({ActionType::PlayPause});
      }
      if (ImGui::MenuItem("Increase speed", "]")) {
        actions.push_back({ActionType::PlaybackSpeedIncrease});
      }
      if (ImGui::MenuItem("Decrease speed", "[")) {
        actions.push_back({ActionType::PlaybackSpeedDecrease});
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Help")) {
      ImGui::MenuItem("(Help menu)", NULL, false, false);
      if (ImGui::MenuItem("Help", "", false)) {
        actions.push_back({ActionType::ShowHelp});
      }
      if (ImGui::MenuItem("Logs", "", false)) {
        actions.push_back({ActionType::ShowLogs});
      }
      if (ImGui::MenuItem("ImGui Demo", NULL, false)) {
        actions.push_back({ActionType::ToggleImGuiDemo});
      }
      ImGui::Separator();
      if (ImGui::MenuItem("About", NULL, false)) {
        actions.push_back({ActionType::ShowAbout});
      }
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }
  return actions;
}
