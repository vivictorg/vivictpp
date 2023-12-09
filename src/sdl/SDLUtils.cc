// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "sdl/SDLUtils.hh"
#include "logging/Logging.hh"
#include <SDL3/SDL_pixels.h>

std::atomic<int> vivictpp::sdl::SDLInitializer::instanceCount(0);

vivictpp::sdl::SDLInitializer::SDLInitializer(bool enableAudio,
                                              bool enableOpenGl) {
  if (instanceCount++ == 0) {
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
    uint32_t flags = SDL_INIT_VIDEO | (enableAudio ? SDL_INIT_AUDIO : 0) |
                     (enableOpenGl ? SDL_WINDOW_OPENGL : 0);
    if (!SDL_Init(flags)) {
      throw SDLException("Failed to initialize SDL");
    }
  }
}

vivictpp::sdl::SDLInitializer::~SDLInitializer() {
  if (--instanceCount == 0) {
    SDL_Quit();
  }
}

// vivictpp::sdl::SDLTexture::SDLTexture() {}

vivictpp::sdl::SDLTexture::SDLTexture(SDL_Renderer *renderer, int w, int h,
                                      SDL_PixelFormat pixelFormat)
    : texturePtr(createTexture(renderer, w, h, pixelFormat)),
      pixelFormat(pixelFormat) {}

void vivictpp::sdl::SDLTexture::update(const vivictpp::libav::Frame &frame) {
  if (pixelFormat == SDL_PIXELFORMAT_YV12) {
    SDL_UpdateYUVTexture(texturePtr.get(), nullptr, frame->data[0],
                         frame->linesize[0], frame->data[1], frame->linesize[1],
                         frame->data[2], frame->linesize[2]);
  } else {
    SDL_UpdateNVTexture(texturePtr.get(), nullptr, frame->data[0],
                        frame->linesize[0], frame->data[1], frame->linesize[1]);
  }
}

vivictpp::sdl::SDLWindow vivictpp::sdl::createWindow(int width, int height,
                                                     int flags) {
  auto window = std::unique_ptr<SDL_Window, std::function<void(SDL_Window *)>>(
      SDL_CreateWindow("Vivict++", width, height, flags), SDL_DestroyWindow);
  if (!window) {
    throw SDLException("Failed to create window");
  }
  return window;
}

vivictpp::sdl::SDLRenderer vivictpp::sdl::createRenderer(SDL_Window *window) {
  auto renderer =
      std::unique_ptr<SDL_Renderer, std::function<void(SDL_Renderer *)>>(
          SDL_CreateRenderer(window, nullptr), SDL_DestroyRenderer);
  if (!renderer) {
    throw SDLException("Failed to create renderer");
  }
  SDL_SetRenderVSync(renderer.get(), 1);

  SDL_PropertiesID props = SDL_GetRendererProperties(renderer.get());
  SDL_PixelFormat *format = (SDL_PixelFormat *)SDL_GetPointerProperty(
      props, SDL_PROP_RENDERER_TEXTURE_FORMATS_POINTER, nullptr);
  while (format && *format != SDL_PIXELFORMAT_UNKNOWN) {
    spdlog::info("Texture format: {}", SDL_GetPixelFormatName(*format));
    format++;
  }

  return renderer;
}

vivictpp::sdl::TexturePtr
vivictpp::sdl::createTexture(SDL_Renderer *renderer, int w, int h,
                             SDL_PixelFormat pixelFormat) {
  auto texture = std::shared_ptr<SDL_Texture>(
      SDL_CreateTexture(renderer, pixelFormat, SDL_TEXTUREACCESS_STREAMING, w,
                        h),
      SDL_DestroyTexture);
  if (!texture) {
    throw SDLException("Failed to create texture");
  }
  return texture;
}

std::unique_ptr<SDL_Cursor, std::function<void(SDL_Cursor *)>>
vivictpp::sdl::createPanCursor() {
  auto cursor = std::unique_ptr<SDL_Cursor, std::function<void(SDL_Cursor *)>>(
      SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_MOVE), SDL_DestroyCursor);
  if (!cursor) {
    throw SDLException("Failed to create cursor");
  }
  return cursor;
}

std::unique_ptr<SDL_Cursor, std::function<void(SDL_Cursor *)>>
vivictpp::sdl::createHandCursor() {
  auto cursor = std::unique_ptr<SDL_Cursor, std::function<void(SDL_Cursor *)>>(
      SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER), SDL_DestroyCursor);
  if (!cursor) {
    throw SDLException("Failed to create cursor");
  }
  return cursor;
}
