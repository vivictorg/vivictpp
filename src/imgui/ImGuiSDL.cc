// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "imgui/ImGuiSDL.hh"
#include "imgui.h"
#include "imgui/Events.hh"
#include "imgui/Fonts.hh"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"
#include "ui/FontSize.hh"
#include "platform_folders.h"
#include "fmt/core.h"
#include <filesystem>
#include <memory>
extern "C" {
#include "SDL.h"
}
#include "logging/Logging.hh"


static ImVec4 CLEAR_COLOR = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);

vivictpp::imgui::ImGuiSDL::ImGuiSDL(const Settings &settings):
  sdlInitializer(false),
  windowPtr(vivictpp::sdl::createWindow(
           windowWidth, windowHeight, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI)),
  window(windowPtr.get()),
  rendererPtr(vivictpp::sdl::createRenderer(window,
                                            SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED)),
  renderer(rendererPtr.get()),
  iniFilename(std::filesystem::path(fmt::format("{}/vivictpp/imgui.ini", sago::getConfigHome())).make_preferred()),
  iniFilenameStr(iniFilename.string())
{
  IMGUI_CHECKVERSION();

  ImGui::CreateContext();

  ImGuiIO& io = ImGui::GetIO();
  iniFilename.make_preferred();
  // Create vivictpp config directory
  std::filesystem::create_directories(iniFilename.parent_path());
  spdlog::debug("Using ini file: {}", iniFilename.string());
  io.IniFilename = iniFilenameStr.c_str();

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

//  ui::initIconTextures(renderer);

  int rw = 0, rh = 0;
  SDL_GetRendererOutputSize(renderer, &rw, &rh);
  scaleRenderer = rw != windowWidth;;
  if(scaleRenderer) {
    float widthScale = (float)rw / (float) windowWidth;
    float heightScale = (float)rh / (float) windowHeight;

    if(widthScale != heightScale) {
      fprintf(stderr, "WARNING: width scale != height scale\n");
    }

    SDL_RenderSetScale(renderer, widthScale, heightScale);
  }

  // Setup Platform/Renderer backends
  ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
  ImGui_ImplSDLRenderer_Init(renderer);
  // Dont't scale fonts in relation to dpi if we are scaling the renderer
  vivictpp::ui::FontSize::setScaling(!scaleRenderer && !settings.disableFontAutoScaling,
                                         settings.baseFontSize / 13.0f);
  initFonts();

}

void vivictpp::imgui::ImGuiSDL::updateFontSettings(const Settings &settings) {
  ImGui::GetIO().Fonts->Clear();
  ImGui_ImplSDLRenderer_DestroyFontsTexture();
  vivictpp::ui::FontSize::setScaling(!scaleRenderer && !settings.disableFontAutoScaling,
                                     settings.baseFontSize / 13.0f);
  initFonts();
}

vivictpp::imgui::ImGuiSDL::~ImGuiSDL() {
  ImGui_ImplSDLRenderer_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
}

void vivictpp::imgui::ImGuiSDL::newFrame() {
  ImGui_ImplSDLRenderer_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();
}

void vivictpp::imgui::ImGuiSDL::render() {
 ImGui::Render();
 SDL_SetRenderDrawColor(renderer, (Uint8)(CLEAR_COLOR.x * 255), (Uint8)(CLEAR_COLOR.y * 255), (Uint8)(CLEAR_COLOR.z * 255), (Uint8)(CLEAR_COLOR.w * 255));
 SDL_RenderClear(renderer);
 ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
 SDL_RenderPresent(renderer);
 ImGui::EndFrame();
}

void vivictpp::imgui::ImGuiSDL::updateTextures(const ui::DisplayState &displayState) {
  videoTextures.update(renderer, displayState);
}

void vivictpp::imgui::ImGuiSDL::fitWindowToTextures() {
  SDL_DisplayMode DM;
  SDL_GetCurrentDisplayMode(0, &DM);
  SDL_SetWindowSize(window, std::min(DM.w, videoTextures.nativeResolution.w), std::min(DM.h, 20 + videoTextures.nativeResolution.h));
}

bool vivictpp::imgui::ImGuiSDL::toggleFullscreen() {
  fullscreen = !fullscreen;
  if (fullscreen) {
    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
  } else {
    SDL_SetWindowFullscreen(window, 0);
  }
  return fullscreen;
}

bool vivictpp::imgui::ImGuiSDL::isWindowClose(SDL_Event &event) {
  return event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window);
}

std::vector<std::shared_ptr<vivictpp::imgui::Event>> vivictpp::imgui::ImGuiSDL::handleEvents() {
  ImGuiIO &io = ImGui::GetIO();
  std::vector<std::shared_ptr<vivictpp::imgui::Event>> events;
  SDL_Event event;
  while (SDL_PollEvent(&event))
  {
    ImGui_ImplSDL2_ProcessEvent(&event);
    if (event.type == SDL_QUIT || isWindowClose(event)) {
      events.push_back(std::make_shared<vivictpp::imgui::Quit>());
      return events;
    }
    else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
      windowWidth = event.window.data1;
      windowHeight = event.window.data2;
      events.push_back(std::make_shared<vivictpp::imgui::WindowSizeChange>());
    }
    else if (event.type == SDL_MOUSEMOTION) {
      SDL_MouseMotionEvent mouseEvent = event.motion;
      events.push_back(std::make_shared<vivictpp::imgui::MouseMotion>(mouseEvent.x, mouseEvent.y));
    }
    else if (event.type == SDL_KEYDOWN && !io.WantCaptureKeyboard) {
      SDL_KeyboardEvent kbe = event.key;
      SDL_Keymod modState = SDL_GetModState();
      events.push_back(std::make_shared<vivictpp::imgui::KeyEvent>(
                         std::string(SDL_GetKeyName(kbe.keysym.sym)),
                         !!(modState & KMOD_SHIFT),
                         !!(modState & KMOD_CTRL),
                         !!(modState & KMOD_ALT)));

    }
  }
  return events;
}
