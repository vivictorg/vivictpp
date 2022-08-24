// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef SDL_SDLUTILS_HH
#define SDL_SDLUTILS_HH

extern "C" {
#include <SDL.h>
}

#include <functional>
#include <exception>
#include <memory>
#include <atomic>
#include <string>
#include <stdexcept>

namespace vivictpp {
namespace sdl {

class SDLException : public std::runtime_error {
 public:
 SDLException(std::string msg): std::runtime_error(msg + std::string(": ") +
                                                   std::string(SDL_GetError())) {}
};

class SDLInitializer {
 private:
  static std::atomic<int> instanceCount;
 public:
  SDLInitializer();
  ~SDLInitializer();
};

std::unique_ptr<SDL_Window, std::function<void(SDL_Window *)>>
  createWindow(int width, int height);

std::unique_ptr<SDL_Renderer, std::function<void(SDL_Renderer *)>>
  createRenderer(SDL_Window* window);

std::unique_ptr<SDL_Texture, std::function<void(SDL_Texture *)>>
  createTexture(SDL_Renderer* renderer, int w, int h);

std::unique_ptr<SDL_Cursor, std::function<void(SDL_Cursor *)>>
  createHandCursor();

std::unique_ptr<SDL_Cursor, std::function<void(SDL_Cursor *)>>
  createPanCursor();

}  // sdl
}  // vivictpp

#endif // SDL_SDLUTILS_HH
