// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef SDL_SDLUTILS_HH
#define SDL_SDLUTILS_HH

#include "SDL_video.h"
extern "C" {
#include "SDL.h"
}

#include <atomic>
#include <exception>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>

#include "libav/Frame.hh"

namespace vivictpp {
namespace sdl {

class SDLException : public std::runtime_error {
public:
  SDLException(std::string msg)
      : std::runtime_error(msg + std::string(": ") +
                           std::string(SDL_GetError())) {}
};

class SDLInitializer {
private:
  static std::atomic<int> instanceCount;

public:
  SDLInitializer(bool enableAudio = true);
  ~SDLInitializer();
};

// typedef std::shared_ptr<SDL_Texture, std::function<void(SDL_Texture *)>>
// TexturePtr;
typedef std::shared_ptr<SDL_Texture> TexturePtr;

class SDLTexture {
public:
  SDLTexture() {}
  SDLTexture(SDL_Renderer *renderer, int w, int h,
             SDL_PixelFormatEnum pixelFormat);
  void update(const vivictpp::libav::Frame &frame);
  bool operator!() const { return !texturePtr; }
  TexturePtr &operator->() { return texturePtr; }
  SDL_Texture *get() { return texturePtr.get(); }

private:
  TexturePtr texturePtr;
  SDL_PixelFormatEnum pixelFormat;
};

typedef std::unique_ptr<SDL_Window, std::function<void(SDL_Window *)>>
    SDLWindow;
typedef std::unique_ptr<SDL_Renderer, std::function<void(SDL_Renderer *)>>
    SDLRenderer;

SDLWindow createWindow(int width, int height, int flags = SDL_WINDOW_RESIZABLE);

SDLRenderer createRenderer(SDL_Window *window, int flags = 0);

TexturePtr
createTexture(SDL_Renderer *renderer, int w, int h,
              SDL_PixelFormatEnum pixelFormat = SDL_PIXELFORMAT_NV12);

std::unique_ptr<SDL_Cursor, std::function<void(SDL_Cursor *)>>
createHandCursor();

std::unique_ptr<SDL_Cursor, std::function<void(SDL_Cursor *)>>
createPanCursor();

} // namespace sdl
} // namespace vivictpp

#endif // SDL_SDLUTILS_HH
