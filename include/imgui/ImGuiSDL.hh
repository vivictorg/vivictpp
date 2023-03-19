#ifndef VIVICTPP_IMGUI_IMGUISDL_HH_
#define VIVICTPP_IMGUI_IMGUISDL_HH_

#include "imgui/Events.hh"
#include "sdl/SDLUtils.hh"
#include "ui/VideoTextures.hh"
#include <vector>
#include <memory>


namespace vivictpp::imgui {

class ImGuiSDL {
private:
  int windowWidth{1280};
  int windowHeight{720};
  sdl::SDLInitializer sdlInitializer;
  sdl::SDLWindow windowPtr;
  SDL_Window *window;
  sdl::SDLRenderer rendererPtr;
  SDL_Renderer *renderer;
  vivictpp::ui::VideoTextures videoTextures;
  bool fullscreen{false};
public:
  ImGuiSDL();
  ~ImGuiSDL();
  void newFrame();
  void updateTextures(const ui::DisplayState &displayState);
  vivictpp::ui::VideoTextures &getVideoTextures() { return videoTextures; }
  void fitWindowToTextures();
  bool isWindowClose(SDL_Event &event);
  std::vector<std::shared_ptr<Event>> handleEvents();
  void render();
  void toggleFullscreen();
};

}  // namespace vivictpp::imgui


#endif /* VIVICTPP_IMGUI_IMGUISDL_HH_ */
