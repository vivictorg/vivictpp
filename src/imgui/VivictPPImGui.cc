// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "imgui/VivictPPImGui.hh"

#include "SDL_video.h"
#include "Settings.hh"
#include "imgui.h"
#include "imgui/Events.hh"
#include "imgui_internal.h"
#include "VivictPPConfig.hh"
#include "sdl/SDLUtils.hh"
#include "spdlog/logger.h"
#include "time/TimeUtils.hh"
#include "ui/DisplayState.hh"
#include "imgui/Colors.hh"
#include "imgui/Fonts.hh"
#include "imgui/Help.hh"
#include "imgui/About.hh"
//#include "imgui/Fonts.hh"
#include "ImGuiFileDialog.h"
#include <map>
#include <memory>

//ImU32 transparentBg = ImGui::ColorConvertFloat4ToU32({0.0f, 0.0f, 0.0f, 0.4f});

// https://github.com/ocornut/imgui/issues/3379`
bool scrollWhenDraggingOnVoid(const ImVec2& delta, ImGuiMouseButton mouse_button)
{
  // blow
  ImGuiContext& g = *ImGui::GetCurrentContext();
  ImGuiWindow* window = g.CurrentWindow;
  ImGuiID id = window->GetID("##scrolldraggingoverlay");
  ImGui::KeepAliveID(id);
  bool hovered = false;
  bool held = false;
  bool scrolled = false;
  ImGuiButtonFlags button_flags = (mouse_button == 0) ? ImGuiButtonFlags_MouseButtonLeft : (mouse_button == 1) ? ImGuiButtonFlags_MouseButtonRight : ImGuiButtonFlags_MouseButtonMiddle;
  if (g.HoveredId == 0) // If nothing hovered so far in the frame (not same as IsAnyItemHovered()!)
    ImGui::ButtonBehavior(window->Rect(), id, &hovered, &held, button_flags);
  if (held && delta.x != 0.0f) {
    ImGui::SetScrollX(window, window->Scroll.x - delta.x);
    scrolled = true;
  }
  if (held && delta.y != 0.0f) {
    ImGui::SetScrollY(window, window->Scroll.y - delta.y);
    scrolled = true;
  }
  return scrolled;
}

void vivictpp::imgui::VideoWindow::onZoomChange(const Resolution &nativeResolution, const ui::Zoom &zoom) {
  ImVec2 oldVideoSize = videoSize;

  videoSize = {nativeResolution.w * zoom.multiplier(),
    nativeResolution.h * zoom.multiplier()};
  if (videoSize.x > size.x ||
      videoSize.y > size.y) {
    // Adjust scroll so center of picture stays the same
    // TODO: Take 'pad' into account maybe
    ImVec2 center = { (scroll.x + size.x / 2) / oldVideoSize.x,
      (scroll.y + size.y / 2) / oldVideoSize.y };
    scroll = { center.x * videoSize.x - size.x / 2,
      center.y * videoSize.y - size.y / 2 };
    scrollUpdated = true;
  } else {
    if (scroll.x != 0.0 || scroll.y != 0.0) {
      scroll = { 0, 0};
      scrollUpdated = true;
    }
  }
}

void vivictpp::imgui::VideoWindow::onScroll(const ImVec2 &scrollDelta) {
  // TODO: Limit scroll
  float maxScrollX = std::max(0.0f, videoSize.x - size.x);
  float maxScrollY = std::max(0.0f, videoSize.y - size.y);
  scroll = {std::clamp(scroll.x - scrollDelta.x, 0.0f, maxScrollX),
    std::clamp(scroll.y - scrollDelta.y, 0.0f, maxScrollY)};
  scrollUpdated = true;
}

float alignForWidthPos(float width, float alignment = 0.5f, float offset = 0.0f) {
  //   const ImGuiViewport* viewport = ImGui::GetMainViewport();
  // ImVec2 work_size = viewport->WorkSize;
  float avail = ImGui::GetContentRegionAvail().x;
  float off = (avail - width) * alignment;
  return ImGui::GetCursorPosX() + std::max(0.0f, off) + offset;
}

void alignForWidth(float width, float alignment = 0.5f, float offset = 0.0f)
{
  ImGui::SetCursorPosX(alignForWidthPos(width, alignment, offset));
}

void vivictpp::imgui::VideoWindow::draw(vivictpp::ui::VideoTextures &videoTextures, const ui::DisplayState &displayState) {
  const ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImVec2 work_size = viewport->WorkSize;
  ImVec2 work_pos =  viewport->WorkPos;
  pos = {0,work_pos.y};
  size = work_size;
  ImGui::SetNextWindowPos(pos);
  ImGui::SetNextWindowSize(size);

  float scaleFactor = displayState.zoom.multiplier();
  if (displayState.fitToScreen) {
    scaleFactor *= std::min(work_size.x / videoTextures.nativeResolution.w, work_size.y / videoTextures.nativeResolution.h);
  }

  ImVec2 scaledVideoSize = {videoTextures.nativeResolution.w * scaleFactor , videoTextures.nativeResolution.h * scaleFactor};
  videoSize = scaledVideoSize;
  ImGui::SetNextWindowContentSize({std::max(work_size.x, scaledVideoSize.x), std::max(work_size.y, scaledVideoSize.y)});
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

  bool myBool2;
  if (ImGui::Begin("Vivict++", &myBool2,  ImGuiWindowFlags_AlwaysAutoResize |
                   ImGuiWindowFlags_NoDecoration |
                   ImGuiWindowFlags_NoSavedSettings |
                   ImGuiWindowFlags_NoFocusOnAppearing |
                   ImGuiWindowFlags_NoNav |
                   ImGuiWindowFlags_NoBringToFrontOnFocus |
                   ImGuiWindowFlags_NoTitleBar |
                   ImGuiWindowFlags_NoScrollbar
        )) {
    ImVec2 viewSize = work_size;

    if (scrollUpdated) {
      ImGui::SetScrollX(scroll.x);
      ImGui::SetScrollY(scroll.y);
      scrollUpdated = false;
    } else {
      scroll = {ImGui::GetScrollX(), ImGui::GetScrollY()};
    }
    float scrollX = scroll.x;
    float scrollY = scroll.y;

    ImVec2 pad; // = {viewport->Size.x - work_size.x, viewport->Size.y - work_pos.y};
    if (scaledVideoSize.x < viewSize.x) {
      pad.x += std::floor((viewSize.x - scaledVideoSize.x) / 2);
    }
    if (scaledVideoSize.y < viewSize.y) {
      pad.y += std::floor((viewSize.y - scaledVideoSize.y) / 2);
    }
    videoPos = pad;

    float splitX = std::clamp(ImGui::GetMousePos().x, pad.x, pad.x + scaledVideoSize.x);
    ImVec2 drawPos = {pad.x - scrollX, pad.y - scrollY}; //cursorPos;
    ImVec2 uvMin(0, 0);

    ImVec2 uvMax((scrollX + splitX - pad.x) / scaledVideoSize.x ,1);
    ImVec2 p2(pad.x + scaledVideoSize.x - scrollX, pad.y + scaledVideoSize.y - scrollY);
    ImGui::GetWindowDrawList()->AddImage((void*)(intptr_t) videoTextures.leftTexture.get(),
                                         drawPos, p2 /*, uvMin, uvMax*/);

    if (!displayState.splitScreenDisabled) {
      // TODO: Should take into account size of video texture here
      drawPos.x = splitX;
      uvMin.x = uvMax.x;
      uvMax.x = 1.0; //(scrollX + viewSize.x) / scaledVideoSize.x;
      ImGui::GetWindowDrawList()->AddImage((void*)(intptr_t) videoTextures.rightTexture.get(),
                                           drawPos, p2, uvMin, uvMax);
      ImGui::GetWindowDrawList()->AddLine({splitX, pad.y}, {splitX, pad.y + scaledVideoSize.y}, 0x80FFFFFF, 0.5);
    }
  }
  ImGui::End();
  ImGui::PopStyleVar(2);
}

vivictpp::imgui::VivictPPImGui::VivictPPImGui(VivictPPConfig vivictPPConfig):
  settings(vivictpp::loadSettings()),
  imGuiSDL(settings),
  videoPlayback(vivictPPConfig),
  settingsDialog(settings)
{
  displayState.splitScreenDisabled = vivictPPConfig.sourceConfigs.size() < 2;
  if (displayState.splitScreenDisabled) {
    displayState.splitPercent = 100;
  }
  if (videoPlayback.getPlaybackState().ready) {
    displayState.updateFrames(videoPlayback.getVideoInputs().firstFrames());
    displayState.updateMetadata(videoPlayback.getVideoInputs().metadata());
    imGuiSDL.updateTextures(displayState);
    imGuiSDL.fitWindowToTextures();
  }
}

void vivictpp::imgui::VivictPPImGui::run() {

  while (!done) {
    handleActions(handleEvents(imGuiSDL.handleEvents()));

    imGuiSDL.newFrame();
    if (!displayState.fullscreen) {
      handleActions(mainMenu.draw(videoPlayback.getPlaybackState(), displayState));
    }

    handleActions(fileDialog.draw());

    if (videoPlayback.getPlaybackState().ready) {
      int64_t tNextPresent = tLastPresent + (int64_t) (1e6 * ImGui::GetIO().DeltaTime);
      if (videoPlayback.checkAdvanceFrame(tNextPresent)) {
        displayState.updateFrames(videoPlayback.getVideoInputs().firstFrames());
        imGuiSDL.updateTextures(displayState);
      }
      displayState.pts = videoPlayback.getPlaybackState().pts;
      displayState.isPlaying = videoPlayback.isPlaying();
      videoWindow.draw(imGuiSDL.getVideoTextures(), displayState);
    }

    handleActions(controls.draw(videoPlayback.getPlaybackState(), displayState));
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

vivictpp::imgui::Action vivictpp::imgui::VivictPPImGui::handleKeyEvent(const vivictpp::imgui::KeyEvent &keyEvent) {
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
      return {vivictpp::imgui::ToggleFullscreen};
    case 'T':
      return {vivictpp::imgui::ToggleDisplayTime};
      break;
    case 'D':
      if (keyEvent.shift) return {vivictpp::imgui::ToggleImGuiDemo};
      return {vivictpp::imgui::ToggleDisplayMetadata};
    case 'O':
      if (keyEvent.ctrl && keyEvent.shift) return {vivictpp::imgui::ShowFileDialogRight};
      else if (keyEvent.ctrl) return {vivictpp::imgui::ShowFileDialogLeft};
      break;
    case 'P':
      return {vivictpp::imgui::ToggleDisplayPlot};
    case 'S':
      if (keyEvent.ctrl && keyEvent.alt) return {vivictpp::imgui::ShowSettingsDialog};
      else if (keyEvent.noModifiers()) return {vivictpp::imgui::ToggleFitToScreen};
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

std::vector<vivictpp::imgui::Action> vivictpp::imgui::VivictPPImGui::handleEvents(std::vector<std::shared_ptr<vivictpp::imgui::Event>> events) {
  std::vector<Action> actions;
  for (auto &event : events) {
    if (std::dynamic_pointer_cast<vivictpp::imgui::Quit>(event)) {
      actions.push_back({vivictpp::imgui::ActionQuit});
    } else if (std::dynamic_pointer_cast<vivictpp::imgui::WindowSizeChange>(event)) {
      // Do nothing
    } else  if (auto mouseMotion = std::dynamic_pointer_cast<vivictpp::imgui::MouseMotion>(event)) {
      if (!displayState.splitScreenDisabled) {
          displayState.splitPercent = 100.0 * std::clamp((mouseMotion->x - videoWindow.getVideoPos().x) / videoWindow.getVideoSize().x, 0.0f, 1.0f);
      }
    } else if (auto keyEvent = std::dynamic_pointer_cast<vivictpp::imgui::KeyEvent>(event)) {
      actions.push_back(handleKeyEvent(*keyEvent.get()));
    }
  }
  return actions;
}

void vivictpp::imgui::VivictPPImGui::handleActions(std::vector<vivictpp::imgui::Action> actions) {
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
        videoWindow.onZoomChange(imGuiSDL.getVideoTextures().nativeResolution, displayState.zoom);
        break;
      case ActionType::ZoomOut:
        displayState.zoom.decrement();
        videoWindow.onZoomChange(imGuiSDL.getVideoTextures().nativeResolution, displayState.zoom);
        break;
      case ActionType::ZoomReset:
        displayState.zoom.set(0);
        videoWindow.onZoomChange(imGuiSDL.getVideoTextures().nativeResolution, displayState.zoom);
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
        fileDialog.openLeft();
        break;
      case ActionType::ShowFileDialogRight:
        if (videoPlayback.getPlaybackState().hasLeftSource) {
          fileDialog.openRight();
        }
        break;
      case ActionType::OpenFileLeft:
        videoPlayback.setLeftSource(action.file);
        displayState.updateFrames(videoPlayback.getVideoInputs().firstFrames());
        displayState.updateMetadata(videoPlayback.getVideoInputs().metadata());
        imGuiSDL.updateTextures(displayState);
        imGuiSDL.fitWindowToTextures();
        break;
      case ActionType::OpenFileRight:
        videoPlayback.setRightSource(action.file);
        displayState.splitScreenDisabled = false;
        displayState.updateFrames(videoPlayback.getVideoInputs().firstFrames());
        displayState.updateMetadata(videoPlayback.getVideoInputs().metadata());
        imGuiSDL.updateTextures(displayState);
        imGuiSDL.fitWindowToTextures();
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
        vivictpp::saveSettings(settingsDialog.getSettings());
        break;
      default:
        ;
      }
    }
}
