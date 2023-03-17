#include "imgui/ImGuiSDL.hh"
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"

static ImVec4 CLEAR_COLOR = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);

vivictpp::imgui::ImGuiSDL::ImGuiSDL():
  sdlInitializer(false),
  windowPtr(vivictpp::sdl::createWindow(
           windowWidth, windowHeight, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI)),
  window(windowPtr.get()),
  rendererPtr(vivictpp::sdl::createRenderer(window,
                                            SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED)),
  renderer(rendererPtr.get())
{
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  // Setup Platform/Renderer backends
  ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
  ImGui_ImplSDLRenderer_Init(renderer);
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
}

void vivictpp::imgui::ImGuiSDL::updateTextures(const ui::DisplayState &displayState) {
  videoTextures.update(renderer, displayState);
}

void vivictpp::imgui::ImGuiSDL::fitWindowToTextures() {
  SDL_SetWindowSize(window, videoTextures.nativeResolution.w, videoTextures.nativeResolution.h);
}

void vivictpp::imgui::ImGuiSDL::toggleFullscreen() {
  fullscreen = !fullscreen;
  if (fullscreen) {
    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
  } else {
    SDL_SetWindowFullscreen(window, 0);
  }
}

vivictpp::imgui::Event mouseMotionEvent(int x, int y) {
  vivictpp::imgui::Event event;
  event.type = vivictpp::imgui::EventType::MouseMotion;
  event.mouseMotion = {x, y};
  return event;
}

static vivictpp::imgui::Event QUIT = {vivictpp::imgui::EventType::Quit, {0}};
static vivictpp::imgui::Event WINDOW_SIZE_CHANGE = {vivictpp::imgui::EventType::WindowSizeChange, {0}};

bool vivictpp::imgui::ImGuiSDL::isWindowClose(SDL_Event &event) {
  return event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window);
}

std::vector<vivictpp::imgui::Event> vivictpp::imgui::ImGuiSDL::handleEvents() {
  std::vector<vivictpp::imgui::Event> events;
  SDL_Event event;
  while (SDL_PollEvent(&event))
  {
    ImGui_ImplSDL2_ProcessEvent(&event);
    if (event.type == SDL_QUIT || isWindowClose(event)) {
      events.push_back(QUIT);
      return events;
    }
    if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
      windowWidth = event.window.data1;
      windowHeight = event.window.data2;
      events.push_back(WINDOW_SIZE_CHANGE);
    }
    if (event.type == SDL_MOUSEMOTION) {
      SDL_MouseMotionEvent mouseEvent = event.motion;
      events.push_back(mouseMotionEvent(mouseEvent.x, mouseEvent.y));
    }
  }
  return events;
}
