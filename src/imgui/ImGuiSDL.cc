// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "imgui/ImGuiSDL.hh"
#include "fmt/core.h"
#include "imgui.h"
#include "imgui/Events.hh"
#include "imgui/Fonts.hh"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include "platform_folders.h"
#include "ui/FontSize.hh"
#include <filesystem>
#include <memory>
extern "C" {
#include <SDL3/SDL.h>
}
#include "libs/implot/implot.h"
#include "logging/Logging.hh"

static ImVec4 CLEAR_COLOR = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);

vivictpp::imgui::ImGuiSDL::ImGuiSDL(const Settings &settings)
    : sdlInitializer(false),
      windowPtr(vivictpp::sdl::createWindow(windowWidth, windowHeight,
                                            SDL_WINDOW_RESIZABLE |
                                                SDL_WINDOW_HIGH_PIXEL_DENSITY)),
      window(windowPtr.get()),
      rendererPtr(vivictpp::sdl::createRenderer(
          window, SDL_PROP_RENDERER_CREATE_PRESENT_VSYNC_NUMBER)),
      renderer(rendererPtr.get()),
      iniFilename(std::filesystem::path(fmt::format("{}/vivictpp/imgui.ini",
                                                    sago::getConfigHome()))
                      .make_preferred()),
      iniFilenameStr(iniFilename.string()), thumbnailTexture(renderer) {
  IMGUI_CHECKVERSION();

  ImGui::CreateContext();
  ImPlot::CreateContext();

  ImGuiIO &io = ImGui::GetIO();
  iniFilename.make_preferred();
  // Create vivictpp config directory
  std::filesystem::create_directories(iniFilename.parent_path());
  spdlog::debug("Using ini file: {}", iniFilename.string());
  io.IniFilename = iniFilenameStr.c_str();

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  //  ui::initIconTextures(renderer);

  int rw = 0, rh = 0;
  SDL_GetCurrentRenderOutputSize(renderer, &rw, &rh);
  scaleRenderer = rw != windowWidth;
  ;
  if (scaleRenderer) {
    float widthScale = (float)rw / (float)windowWidth;
    float heightScale = (float)rh / (float)windowHeight;

    if (widthScale != heightScale) {
      fprintf(stderr, "WARNING: width scale != height scale\n");
    }

    SDL_SetRenderScale(renderer, widthScale, heightScale);
  }

  // Setup Platform/Renderer backends
  ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
  ImGui_ImplSDLRenderer2_Init(renderer);
  // Dont't scale fonts in relation to dpi if we are scaling the renderer
  vivictpp::ui::FontSize::setScaling(!scaleRenderer &&
                                         !settings.disableFontAutoScaling,
                                     settings.baseFontSize / 13.0f);
  initFonts();
}

void vivictpp::imgui::ImGuiSDL::updateFontSettings(const Settings &settings) {
  ImGui::GetIO().Fonts->Clear();
  ImGui_ImplSDLRenderer2_DestroyFontsTexture();
  vivictpp::ui::FontSize::setScaling(!scaleRenderer &&
                                         !settings.disableFontAutoScaling,
                                     settings.baseFontSize / 13.0f);
  initFonts();
}

vivictpp::imgui::ImGuiSDL::~ImGuiSDL() {
  ImGui_ImplSDLRenderer2_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImPlot::DestroyContext();
  ImGui::DestroyContext();
}

void vivictpp::imgui::ImGuiSDL::newFrame() {
  ImGui_ImplSDLRenderer2_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();
}

void vivictpp::imgui::ImGuiSDL::render() {
  ImGui::Render();
  SDL_SetRenderDrawColor(
      renderer, (Uint8)(CLEAR_COLOR.x * 255), (Uint8)(CLEAR_COLOR.y * 255),
      (Uint8)(CLEAR_COLOR.z * 255), (Uint8)(CLEAR_COLOR.w * 255));
  SDL_RenderClear(renderer);
  ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
  SDL_RenderPresent(renderer);
  ImGui::EndFrame();
}

void vivictpp::imgui::ImGuiSDL::updateTextures(
    const ui::DisplayState &displayState) {
  videoTextures.update(renderer, displayState);
}

void vivictpp::imgui::ImGuiSDL::fitWindowToTextures() {
  int displayIndex = SDL_GetWindowDisplayIndex(window);
  auto displayMode = SDL_GetCurrentDisplayMode(displayIndex);
  if (displayMode == nullptr) {
    throw new vivictpp::sdl::SDLException("Failed to get display mode");
  }
  int w,h;
  if (!SDL_GetWindowSize(window, &w, &h)) {
    throw new vivictpp::sdl::SDLException("Failed to get window size");
  }
  int newW = std::min(displayMode->w, std::max(w,videoTextures.nativeResolution.w));
  int newH = std::min(displayMode >h, std::max(h, 20 +videoTextures.nativeResolution.h));
  if (newW > w || newH > h) {
    if (!SDL_SetWindowSize(window, newW, newH)) {
      spdlog::error("Failed to set window size");
    }
  }
}

bool vivictpp::imgui::ImGuiSDL::toggleFullscreen() {
  fullscreen = !fullscreen;
  if (fullscreen) {
    SDL_SetWindowFullscreenMode(window, nullptr);
    SDL_SetWindowFullscreen(window, true);
  } else {
    SDL_SetWindowFullscreen(window, false);
  }
  return fullscreen;
}

bool vivictpp::imgui::ImGuiSDL::isWindowClose(SDL_Event &event) {
  return event.type == SDL_WINDOWEVENT &&
         event.window.event == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
         event.window.windowID == SDL_GetWindowID(window);
}

std::vector<std::shared_ptr<vivictpp::imgui::Event>>
vivictpp::imgui::ImGuiSDL::handleEvents() {
  ImGuiIO &io = ImGui::GetIO();
  std::vector<std::shared_ptr<vivictpp::imgui::Event>> events;
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL2_ProcessEvent(&event);
    if (event.type == SDL_EVENT_QUIT || isWindowClose(event)) {
      events.push_back(std::make_shared<vivictpp::imgui::Quit>());
      return events;
    } else if (event.type == SDL_WINDOWEVENT &&
               event.window.event == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
      windowWidth = event.window.data1;
      windowHeight = event.window.data2;
      events.push_back(std::make_shared<vivictpp::imgui::WindowSizeChange>());
    } else if (event.type == SDL_EVENT_MOUSE_MOTION) {
      SDL_MouseMotionEvent mouseEvent = event.motion;
      events.push_back(std::make_shared<vivictpp::imgui::MouseMotion>(
          mouseEvent.x, mouseEvent.y));
    } else if (event.type == SDL_EVENT_KEY_DOWN && !io.WantCaptureKeyboard) {
      SDL_KeyboardEvent kbe = event.key;
      SDL_Keymod modState = SDL_GetModState();
      events.push_back(std::make_shared<vivictpp::imgui::KeyEvent>(
          std::string(SDL_GetKeyName(kbe.keysym.sym)),
          !!(modState & SDL_KMOD_SHIFT), !!(modState & SDL_KMOD_CTRL),
          !!(modState & SDL_KMOD_ALT)));
    }
  }
  return events;
}
