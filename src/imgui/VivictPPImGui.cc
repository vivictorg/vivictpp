// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "imgui/VivictPPImGui.hh"

#include "Settings.hh"
#include "SourceConfig.hh"
#include "VivictPPConfig.hh"
#include "imgui.h"
#include "imgui/About.hh"
#include "imgui/Events.hh"
#include "imgui/Help.hh"
#include "imgui/Logs.hh"
#include "imgui/Splash.hh"
#include "imgui/VideoWindow.hh"
#include "imgui_internal.h"
#include "libs/implot/implot.h"
#include "time/TimeUtils.hh"
#include <memory>

// ImU32 transparentBg = ImGui::ColorConvertFloat4ToU32({0.0f, 0.0f, 0.0f,
// 0.4f});

vivictpp::imgui::VivictPPImGui::VivictPPImGui(
    const VivictPPConfig &vivictPPConfig)
    : settings(vivictPPConfig.settings), imGuiSDL(settings),
      videoPlayback(vivictPPConfig.sourceConfigs), settingsDialog(settings),
      plotWindow(videoPlayback.getVideoInputs().getLeftVideoIndex(),
                 videoPlayback.getVideoInputs().getRightVideoIndex()) {

  displayState.splitScreenDisabled = vivictPPConfig.sourceConfigs.size() < 2;
  if (displayState.splitScreenDisabled) {
    displayState.splitPercent = 100;
  }
  if (videoPlayback.getPlaybackState().ready) {
    displayState.updateFrames(videoPlayback.getVideoInputs().firstFrames());
    displayState.updateMetadata(videoPlayback.getVideoInputs().metadata());
    displayState.updateDecoderMetadata(
        videoPlayback.getVideoInputs().decoderMetadata());
    imGuiSDL.updateThumbnails(
        videoPlayback.getVideoInputs().getLeftVideoIndex());
    imGuiSDL.updateTextures(displayState);
    imGuiSDL.fitWindowToTextures();
  }
  if (vivictPPConfig.sourceConfigs.size() > 0) {
    leftQualityMetricsLoader.autoloadMetrics(
        vivictPPConfig.sourceConfigs[0].path,
        [this](
            std::shared_ptr<vivictpp::qualitymetrics::QualityMetrics> metrics,
            std::shared_ptr<std::exception> error) {
          this->loadMetricsCallback(
              metrics, error, vivictpp::imgui::ActionType::OpenQualityFileLeft);
        });
  }
  if (vivictPPConfig.sourceConfigs.size() > 1) {
    rightQualityMetricsLoader.autoloadMetrics(
        vivictPPConfig.sourceConfigs[1].path,
        [this](
            std::shared_ptr<vivictpp::qualitymetrics::QualityMetrics> metrics,
            std::shared_ptr<std::exception> error) {
          this->loadMetricsCallback(
              metrics, error,
              vivictpp::imgui::ActionType::OpenQualityFileRight);
        });
  }
}

void drawSplash() {
  const ImGuiViewport *viewport = ImGui::GetMainViewport();
  ImVec2 work_size = viewport->WorkSize;
  //  ImVec2 work_pos =  viewport->WorkPos;
  bool myBool;
  ImVec2 size = work_size;
  ImVec2 textSize = ImGui::CalcTextSize(vivictpp::imgui::SPLASH_TEXT.c_str());
  ImVec2 pos{(size.x - textSize.x) / 2, 0.8f * (size.y - textSize.y) / 2};
  ImGui::SetNextWindowPos(pos);
  ImGui::SetNextWindowSize({10.f + textSize.x, 10.f + textSize.y});
  if (ImGui::Begin(
          "Splash", &myBool,
          ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground |
              ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings |
              ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav |
              ImGuiWindowFlags_NoBringToFrontOnFocus |
              ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar)) {
    //    ImGui::Text("size=%.1f, %.1f textSize=%.1f, %.1f pos=%.1f, %.1f",
    //                size.x, size.y, textSize.x, textSize.y, pos.x, pos.y);
    //    ImGui::SetCursorPosX(pos.x);
    //    ImGui::SetCursorPosY(pos.y);
    ImGui::Text("%s", vivictpp::imgui::SPLASH_TEXT.c_str());
    ImGui::End();
  }
}

void vivictpp::imgui::VivictPPImGui::run() {

  while (!done) {
    std::shared_ptr<vivictpp::qualitymetrics::QualityMetrics> newValue =
        std::atomic_exchange(
            &newLeftQualityMetrics,
            std::shared_ptr<vivictpp::qualitymetrics::QualityMetrics>(nullptr));
    if (newValue) {
      displayState.leftQualityMetrics = newValue;
    }
    newValue = std::atomic_exchange(
        &newRightQualityMetrics,
        std::shared_ptr<vivictpp::qualitymetrics::QualityMetrics>(nullptr));
    if (newValue) {
      displayState.rightQualityMetrics = newValue;
    }
    handleActions(handleEvents(imGuiSDL.handleEvents()));

    imGuiSDL.newFrame();
    if (!displayState.fullscreen) {
      handleActions(
          mainMenu.draw(videoPlayback.getPlaybackState(), displayState));
    }
    handleActions(fileDialog.draw());
    handleActions(qualityFileDialog.draw());
    if (videoPlayback.getPlaybackState().ready) {
      handleActions(controls.draw(videoPlayback.getPlaybackState(),
                                  displayState,
                                  imGuiSDL.getThumbnailTexture()));
      int64_t tNextPresent =
          tLastPresent + (int64_t)(1e6 * ImGui::GetIO().DeltaTime);
      if (videoPlayback.checkAdvanceFrame(tNextPresent)) {
        displayState.updateFrames(videoPlayback.getVideoInputs().firstFrames());
        imGuiSDL.updateTextures(displayState);
      }
      displayState.pts = videoPlayback.getPlaybackState().pts;
      displayState.isPlaying = videoPlayback.isPlaying();
      videoWindow.draw(imGuiSDL.getVideoTextures(), displayState);
      handleActions(plotWindow.draw(displayState));
    } else {
      drawSplash();
    }

    if (displayState.displayImGuiDemo) {
      bool aBool;
      ImGui::ShowDemoWindow(&aBool);
    }
    if (displayState.displayHelp) {
      showHelp(displayState);
    }
    if (displayState.displayAbout) {
      showAbout(displayState);
    }
    if (displayState.displayLogs) {
      showLogs(displayState);
    }
    if (displayState.displaySettingsDialog) {
      handleActions(settingsDialog.draw(displayState));
    }
    imGuiSDL.render();
    if (settingsDialog.isFontSettingsUpdated()) {
      imGuiSDL.updateFontSettings(settingsDialog.getModifiedSettings());
    }
    tLastPresent = vivictpp::time::relativeTimeMicros();
  }
}

int seekDistance(const vivictpp::imgui::KeyEvent &keyEvent) {
  return keyEvent.shift ? (keyEvent.alt ? 600 : 60) : 5;
}

vivictpp::imgui::Action vivictpp::imgui::VivictPPImGui::handleKeyEvent(
    const vivictpp::imgui::KeyEvent &keyEvent) {
  const std::string &key = keyEvent.keyName;
  if (key.length() == 1) {
    switch (key[0]) {
    case 'Q':
      return {vivictpp::imgui::ActionType::ActionQuit};
    case '.':
      if (keyEvent.shift) {
        return {vivictpp::imgui::ActionType::FrameOffsetIncrease};
      } else {
        return {vivictpp::imgui::ActionType::StepForward};
      }
      break;
    case ',':
      if (keyEvent.shift) {
        return {vivictpp::imgui::ActionType::FrameOffsetDecrease};
      } else {
        return {vivictpp::imgui::ActionType::StepBackward};
      }
    case '/':
      return {vivictpp::imgui::ActionType::SeekRelative,
              vivictpp::time::seconds(seekDistance(keyEvent))};
    case 'M':
      return {vivictpp::imgui::ActionType::SeekRelative,
              vivictpp::time::seconds(-1 * seekDistance(keyEvent))};
    case 'U':
      return {vivictpp::imgui::ZoomIn};
    case 'I':
      return {vivictpp::imgui::ZoomOut};
    case '0':
      return {vivictpp::imgui::ZoomReset};
    case 'F':
      if (keyEvent.alt) {
        return {vivictpp::imgui::ToggleFitToScreen};
      } else {
        return {vivictpp::imgui::ToggleFullscreen};
      }
    case 'T':
      return {vivictpp::imgui::ToggleDisplayTime};
      break;
    case 'D':
      if (keyEvent.shift)
        return {vivictpp::imgui::ToggleImGuiDemo};
      return {vivictpp::imgui::ToggleDisplayMetadata};
    case 'O':
      if (keyEvent.isCtrlAlt())
        return {vivictpp::imgui::ShowFileDialogRight};
      else if (keyEvent.isCtrl())
        return {vivictpp::imgui::ShowFileDialogLeft};
      break;
    case 'P':
      if (keyEvent.isCtrlAlt())
        return {vivictpp::imgui::ShowQualityFileDialogRight};
      else if (keyEvent.isCtrl())
        return {vivictpp::imgui::ShowQualityFileDialogLeft};
      return {vivictpp::imgui::ToggleDisplayPlot};
    case 'S':
      if (keyEvent.ctrl && keyEvent.alt)
        return {vivictpp::imgui::ShowSettingsDialog};
      else if (keyEvent.noModifiers())
        return {vivictpp::imgui::ToggleFitToScreen};
      break;
    case '[':
      return {vivictpp::imgui::PlaybackSpeedDecrease};
    case ']':
      return {vivictpp::imgui::PlaybackSpeedIncrease};
    }
  } else {
    if (key == "Space") {
      return {vivictpp::imgui::PlayPause};
    }
  }
  return {vivictpp::imgui::NoAction};
}

std::vector<vivictpp::imgui::Action>
vivictpp::imgui::VivictPPImGui::handleEvents(
    std::vector<std::shared_ptr<vivictpp::imgui::Event>> events) {
  std::vector<Action> actions;
  for (auto &event : events) {
    if (std::dynamic_pointer_cast<vivictpp::imgui::Quit>(event)) {
      actions.push_back({vivictpp::imgui::ActionQuit});
    } else if (std::dynamic_pointer_cast<vivictpp::imgui::WindowSizeChange>(
                   event)) {
      // Do nothing
    } else if (auto mouseMotion =
                   std::dynamic_pointer_cast<vivictpp::imgui::MouseMotion>(
                       event)) {
      if (!displayState.splitScreenDisabled) {
        displayState.splitPercent =
            100.0 * std::clamp((mouseMotion->x - videoWindow.getVideoPos().x) /
                                   videoWindow.getVideoSize().x,
                               0.0f, 1.0f);
      }
    } else if (auto keyEvent =
                   std::dynamic_pointer_cast<vivictpp::imgui::KeyEvent>(
                       event)) {
      actions.push_back(handleKeyEvent(*keyEvent.get()));
    }
  }
  return actions;
}

void vivictpp::imgui::VivictPPImGui::handleActions(
    std::vector<vivictpp::imgui::Action> actions) {
  for (auto &action : actions) {
    switch (action.type) {
    case ActionType::ActionQuit:
      done = true;
      break;
    case ActionType::PlayPause:
      videoPlayback.togglePlaying();
      break;
    case ActionType::ZoomIn:
      displayState.zoom.increment();
      videoWindow.onZoomChange(imGuiSDL.getVideoTextures().nativeResolution,
                               displayState.zoom);
      break;
    case ActionType::ZoomOut:
      displayState.zoom.decrement();
      videoWindow.onZoomChange(imGuiSDL.getVideoTextures().nativeResolution,
                               displayState.zoom);
      break;
    case ActionType::ZoomReset:
      displayState.zoom.set(0);
      videoWindow.onZoomChange(imGuiSDL.getVideoTextures().nativeResolution,
                               displayState.zoom);
      break;
    case ActionType::Seek:
      videoPlayback.seek(action.seek);
      break;
    case ActionType::SeekRelative:
      videoPlayback.seekRelative(action.seek);
      break;
    case ActionType::StepForward:
      videoPlayback.seekRelativeFrame(1);
      break;
    case ActionType::StepBackward:
      videoPlayback.seekRelativeFrame(-1);
      break;
    case ActionType::ToggleFullscreen:
      displayState.fullscreen = imGuiSDL.toggleFullscreen();
      break;
    case ActionType::ToggleFitToScreen:
      displayState.fitToScreen = !displayState.fitToScreen;
      break;
    case ActionType::ToggleImGuiDemo:
      displayState.displayImGuiDemo = !displayState.displayImGuiDemo;
      break;
    case ActionType::ToggleDisplayTime:
      displayState.displayTime = !displayState.displayTime;
      break;
    case ActionType::ToggleDisplayMetadata:
      displayState.displayMetadata = !displayState.displayMetadata;
      break;
    case ActionType::ToggleDisplayPlot:
      displayState.displayPlot = !displayState.displayPlot;
      break;
    case ActionType::Scroll:
      videoWindow.onScroll(action.scroll);
      break;
    case ActionType::PlaybackSpeedIncrease:
      videoPlayback.adjustPlaybackSpeed(1);
      break;
    case ActionType::PlaybackSpeedDecrease:
      videoPlayback.adjustPlaybackSpeed(-1);
      break;
    case ActionType::FrameOffsetDecrease:
      displayState.leftFrameOffset = videoPlayback.deccreaseLeftFrameOffset();
      break;
    case ActionType::FrameOffsetIncrease:
      displayState.leftFrameOffset = videoPlayback.increaseLeftFrameOffset();
      break;
    case ActionType::ShowFileDialogLeft:
      fileDialog.openLeft(displayState.leftVideoMetadata.source);
      break;
    case ActionType::ShowFileDialogRight:
      if (videoPlayback.getPlaybackState().hasLeftSource) {
        fileDialog.openRight(displayState.rightVideoMetadata.source);
      }
      break;
    case ActionType::OpenFileLeft:
      openFile(action);
      break;
    case ActionType::OpenFileRight:
      openFile(action);
      break;
    case ActionType::ShowHelp:
      displayState.displayHelp = !displayState.displayHelp;
      break;
    case ActionType::ShowAbout:
      displayState.displayAbout = !displayState.displayAbout;
      break;
    case ActionType::ShowSettingsDialog:
      displayState.displaySettingsDialog = !displayState.displaySettingsDialog;
      break;
    case ActionType::UpdateSettings:
      settings = settingsDialog.getSettings();
      vivictpp::saveSettings(settings);
      vivictpp::logging::setLogLevels(settings.logLevels);
      break;
    case ActionType::ShowLogs:
      displayState.displayLogs = !displayState.displayLogs;
      break;
    case ActionType::ShowQualityFileDialogLeft:
      qualityFileDialog.openLeft(displayState.leftVideoMetadata.source);
      break;
    case ActionType::ShowQualityFileDialogRight:
      qualityFileDialog.openRight(displayState.rightVideoMetadata.source);
      break;
    case ActionType::OpenQualityFileLeft:
      openQualityFile(action);
      break;
    case ActionType::OpenQualityFileRight:
      openQualityFile(action);
      break;
    default:;
    }
  }
}

void vivictpp::imgui::VivictPPImGui::openFile(
    const vivictpp::imgui::Action &action) {
  std::vector<std::string> hwAccels;
  if (fileDialog.selectedHwAccel() == "auto") {
    hwAccels = settings.hwAccels;
  } else if (fileDialog.selectedHwAccel() != "none") {
    hwAccels.push_back(fileDialog.selectedHwAccel());
  }
  std::vector<std::string> preferredDecoders;
  if (fileDialog.selectedDecoder() == "auto") {
    preferredDecoders = settings.preferredDecoders;
  } else {
    preferredDecoders.push_back(fileDialog.selectedDecoder());
  }
  SourceConfig sourceConfig = {action.file, hwAccels, preferredDecoders,
                               fileDialog.filter(), fileDialog.formatOptions()};
  if (action.type == ActionType::OpenFileLeft) {
    videoPlayback.setLeftSource(sourceConfig);
  } else {
    videoPlayback.setRightSource(sourceConfig);
    displayState.splitScreenDisabled = false;
  }
  displayState.updateFrames(videoPlayback.getVideoInputs().firstFrames());
  displayState.updateMetadata(videoPlayback.getVideoInputs().metadata());
  displayState.updateDecoderMetadata(
      videoPlayback.getVideoInputs().decoderMetadata());
  if (action.type == ActionType::OpenFileLeft) {
    imGuiSDL.updateThumbnails(
        videoPlayback.getVideoInputs().getLeftVideoIndex());
  }
  imGuiSDL.updateTextures(displayState);
  imGuiSDL.fitWindowToTextures();
}

void vivictpp::imgui::VivictPPImGui::openQualityFile(
    const vivictpp::imgui::Action &action) {
  auto callback =
      [this, action](
          std::shared_ptr<vivictpp::qualitymetrics::QualityMetrics> metrics,
          std::shared_ptr<std::exception> error) {
        this->loadMetricsCallback(metrics, error, action);
      };
  if (action.type == ActionType::OpenQualityFileLeft) {
    leftQualityMetricsLoader.loadMetrics(action.file, callback);
  } else if (action.type == ActionType::OpenQualityFileRight) {
    rightQualityMetricsLoader.loadMetrics(action.file, callback);
  }
}

void vivictpp::imgui::VivictPPImGui::loadMetricsCallback(
    std::shared_ptr<vivictpp::qualitymetrics::QualityMetrics> metrics,
    std::shared_ptr<std::exception> error, vivictpp::imgui::Action action) {
  if (error) {
    this->logger->error("Error loading quality file: {}", error->what());
    return;
  }
  if (action.type == ActionType::OpenQualityFileLeft) {
    std::atomic_store(&newLeftQualityMetrics, metrics);
  } else if (action.type == ActionType::OpenQualityFileRight) {
    std::atomic_store(&newRightQualityMetrics, metrics);
  }
}