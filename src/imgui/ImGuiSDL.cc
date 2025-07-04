// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "imgui/ImGuiSDL.hh"
#include "fmt/core.h"
#include "imgui.h"
#include "imgui/Events.hh"
#include "imgui/Fonts.hh"

#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

// #include "imgui_impl_sdlrenderer.h"
#include "imgui_impl_opengl3.h"
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL3/SDL_opengles2.h>
#else
#include <SDL3/SDL_opengl.h>
#endif

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

void listDisplayModes() {
  
  int displayCount{0};
  SDL_DisplayID *displays = SDL_GetDisplays(&displayCount);
  std::cout << "DisplayCount: " << displayCount << "\n";
  while (displayCount--) {
    std::cout << "Display: " << *displays << "\n";
      int count{0}; 
    SDL_DisplayMode** modes;  
    modes = SDL_GetFullscreenDisplayModes(*displays, &count); 
    for (int i = 0; i < count; ++i) { 
      SDL_DisplayMode* Mode = modes[i]; 
      std::cout<< "\nModeIndex "  
        << i << ": " << Mode->w << 'x' << Mode->h 
        << " - " << SDL_GetPixelFormatName(Mode->format)  
        << " - " << Mode->refresh_rate << "FPS";  
    }
    displays++;
  } 
}

vivictpp::imgui::ImGuiSDL::ImGuiSDL(const Settings &settings)
    : sdlInitializer(false),
      // windowPtr(vivictpp::sdl::createWindow(windowWidth, windowHeight,
      //                                       SDL_WINDOW_RESIZABLE |
      //                                       SDL_WINDOW_OPENGL |
      //                                           SDL_WINDOW_HIGH_PIXEL_DENSITY)),
      // window(windowPtr.get()),
      // rendererPtr(vivictpp::sdl::createRenderer(
      //     window)),
      // renderer(rendererPtr.get()),
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

  // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
  // GL ES 2.0 + GLSL 100
  const char *glsl_version = "#version 100";
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
  // GL 3.2 Core + GLSL 150
  const char *glsl_version = "#version 150";
  SDL_GL_SetAttribute(
      SDL_GL_CONTEXT_FLAGS,
      SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
  // GL 3.0 + GLSL 130
  const char *glsl_version = "#version 130";
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

  // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
  SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

  // Create window with graphics context
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 10);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 10);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 10);
  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 2);
  glDisable(GL_DITHER);
  // glGetIntegerv( GL_RED_BITS, &BitSize );
  SDL_WindowFlags window_flags =
      (SDL_WindowFlags)(SDL_WINDOW_OPENGL |
                        SDL_WINDOW_RESIZABLE /*| SDL_WINDOW_ALLOW_HIGHDPI*/);
  window = SDL_CreateWindow("Dear ImGui SDL2+OpenGL3 example", 1280, 720,
                            window_flags);
  windowPtr.reset(window);
  gl_context = SDL_GL_CreateContext(window);
  SDL_GL_MakeCurrent(window, gl_context);
  SDL_GL_SetSwapInterval(1); // Enable vsync

  // Initialize GLEW
  glewExperimental = true; // Needed for core profile
  if (glewInit() != GLEW_OK) {
    // fprintf(stderr, "Failed to initialize GLEW\n");
    // return -1;
    // TODO: Handle error
  }

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  listDisplayModes();
  SDL_DisplayMode const *displayMode0 = SDL_GetCurrentDisplayMode(0);
  SDL_DisplayMode const *displayMode1 = SDL_GetCurrentDisplayMode(1);
 
  //SDL_SetWindowSize(window, displayMode0->w, displayMode0->h);
  //SDL_SetWindowSize(window, displayMode.w, displayMode.h);
  //  ui::initIconTextures(renderer);
  /*

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
  */
  // Setup Platform/Renderer backends

  /*
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);
    */
  scaleRenderer = false;
  ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
  ImGui_ImplOpenGL3_Init(glsl_version);
  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 10);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 10);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 10);

  // Dont't scale fonts in relation to dpi if we are scaling the renderer
  vivictpp::ui::FontSize::setScaling(!scaleRenderer &&
                                         !settings.disableFontAutoScaling,
                                     settings.baseFontSize / 13.0f);
  initFonts();
}

void vivictpp::imgui::ImGuiSDL::updateFontSettings(const Settings &settings) {
  ImGui::GetIO().Fonts->Clear();

  // ImGui_ImplSDLRenderer3_DestroyFontsTexture();
  ImGui_ImplOpenGL3_DestroyFontsTexture();
  vivictpp::ui::FontSize::setScaling(!scaleRenderer &&
                                         !settings.disableFontAutoScaling,
                                     settings.baseFontSize / 13.0f);
  initFonts();
}

vivictpp::imgui::ImGuiSDL::~ImGuiSDL() {

  //  ImGui_ImplSDLRenderer3_Shutdown();
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();
  SDL_GL_DestroyContext(gl_context);
}

void vivictpp::imgui::ImGuiSDL::newFrame() {
  //  ImGui_ImplSDLRenderer3_NewFrame();
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();
}

void vivictpp::imgui::ImGuiSDL::render() {
  ImGui::Render();
  /*
  SDL_SetRenderDrawColor(
      renderer, (Uint8)(CLEAR_COLOR.x * 255), (Uint8)(CLEAR_COLOR.y * 255),
      (Uint8)(CLEAR_COLOR.z * 255), (Uint8)(CLEAR_COLOR.w * 255));
  SDL_RenderClear(renderer);
  ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
  SDL_RenderPresent(renderer);
  ImGui::EndFrame();
  */
  ImGuiIO &io = ImGui::GetIO();
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
  glClearColor(CLEAR_COLOR.x * CLEAR_COLOR.w, CLEAR_COLOR.y * CLEAR_COLOR.w,
               CLEAR_COLOR.z * CLEAR_COLOR.w, CLEAR_COLOR.w);
  glClear(GL_COLOR_BUFFER_BIT);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  SDL_GL_SwapWindow(window);
}

void vivictpp::imgui::ImGuiSDL::updateTextures(
    const ui::DisplayState &displayState) {
  // videoTextures.update(renderer, displayState);
  openGLVideoTextures.update(displayState);
}

void vivictpp::imgui::ImGuiSDL::fitWindowToTextures() {
  int displayIndex = SDL_GetDisplayForWindow(window);
  auto displayMode = SDL_GetCurrentDisplayMode(displayIndex);
  if (displayMode == nullptr) {
    throw new vivictpp::sdl::SDLException("Failed to get display mode");
  }
  int w, h;
  if (!SDL_GetWindowSize(window, &w, &h)) {
    throw new vivictpp::sdl::SDLException("Failed to get window size");
  }
  int newW = std::min(
      displayMode->w,
      std::max(w, openGLVideoTextures.getVideoTextures().nativeResolution.w));
  int newH = std::min(
      displayMode->h,
      std::max(h,
               20 + openGLVideoTextures.getVideoTextures().nativeResolution.h));
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
  return event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
         event.window.windowID == SDL_GetWindowID(window);
}

std::vector<std::shared_ptr<vivictpp::imgui::Event>>
vivictpp::imgui::ImGuiSDL::handleEvents() {
  ImGuiIO &io = ImGui::GetIO();
  std::vector<std::shared_ptr<vivictpp::imgui::Event>> events;
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL3_ProcessEvent(&event);
    if (event.type == SDL_EVENT_QUIT || isWindowClose(event)) {
      events.push_back(std::make_shared<vivictpp::imgui::Quit>());
      return events;
    } else if (event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
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
          std::string(SDL_GetKeyName(kbe.key)), !!(modState & SDL_KMOD_SHIFT),
          !!(modState & SDL_KMOD_CTRL), !!(modState & SDL_KMOD_ALT)));
    }
  }
  return events;
}
