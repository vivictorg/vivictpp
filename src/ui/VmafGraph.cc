// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ui/VmafGraph.hh"
#include "ui/TextTexture.hh"

vivictpp::ui::VmafGraph::VmafGraph(std::vector<vivictpp::vmaf::VmafLog> vmafLogs,
                                   float relativeWidth, float relativeHeight):
  vmafLogs(vmafLogs),
  relativeWidth(relativeWidth),
  relativeHeight(relativeHeight),
  rendererWidth(0),
  rendererHeight(0),
  texture(nullptr),
  logger(vivictpp::logging::getOrCreateLogger("VmafGraph")),
  _empty(true) {
  for (auto vmafLog: vmafLogs) {
    if (!vmafLog.empty()) {
      _empty = false;
      break;
    }
  }
}

void vivictpp::ui::VmafGraph::initTexture(SDL_Renderer *renderer) {
  textureW = (int) (relativeWidth * rendererWidth);
  textureH = (int) (relativeHeight * rendererHeight);
  logger->debug("textureW: {}, textureH: {}", textureW, textureH);
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                              SDL_TEXTUREACCESS_TARGET, textureW, textureH);

  SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
  SDL_SetRenderTarget(renderer, texture);

  SDL_Rect rect = {0, 0, textureW, textureH};
  SDL_RenderSetClipRect(renderer, &rect);
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 64);
  SDL_RenderClear(renderer);
  // Render grid

  SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_NONE);
  for (int i = 0; i<100;  i+=25) {
    int y = textureH - i * textureH / 100;
    SDL_SetRenderDrawColor(renderer, 64, 64, 64, 128);
    SDL_RenderDrawLine(renderer, 0, y, textureW, y);
    TextTexture text(renderer, std::to_string(i), 20);
    text.render(renderer, 0, y - text.height);
  }
  SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);


  SDL_Color colors[] = { {255,0,0,255}, {0,0,255,255} };
  for (size_t j = 0; j < vmafLogs.size() ; j++) {
    const std::vector<float> &vmafValues = vmafLogs[j].getVmafValues();
    if (vmafValues.empty()) {
      continue;
    }
//  int p0(height - (int)(vmafValues[0] * height / 100)),p1(0);
    SDL_Color color = colors[j % 2];
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (int i=0; i<textureW; i++) {
      int i0 = (int)(i * vmafValues.size() / textureW);
      int i1 = std::min((int)((i+1) * vmafValues.size() / textureW), (int) vmafValues.size());
      float vmin(100), vmax(0);
      for (int j=i0; j<i1; j++) {
        vmin = std::min(vmin, vmafValues[j]);
        vmax = std::max(vmax, vmafValues[j]);
      }
      int p0 = textureH - (int) (vmax * textureH / 100);
      int p1 = textureH - (int) (vmin * textureH / 100);
      SDL_RenderDrawLine(renderer, i, p0, i, p1);
    }
  }
//  SDL_RenderSetClipRect(renderer, nullptr);
  SDL_SetRenderTarget(renderer, nullptr);
}

void vivictpp::ui::VmafGraph::render(SDL_Renderer *renderer,
                                     double pts,
                                     double startTime,
                                     double duration) {
  int oldRendererWidth = rendererWidth;
  int oldRendererHeight = rendererHeight;
  SDL_GetRendererOutputSize(renderer, &rendererWidth, &rendererHeight);
  if (oldRendererHeight != rendererHeight || oldRendererWidth != rendererWidth) {
    logger->debug("Output size changed: old: {}x{} new: {}x{}",
                 oldRendererWidth, oldRendererHeight, rendererWidth, rendererHeight);
    SDL_DestroyTexture(texture);
    texture = nullptr;
  }
  if (!texture) {
    initTexture(renderer);
  }
  
  SDL_Rect rect = {0, rendererHeight - textureH - 2, textureW, textureH};

  SDL_RenderCopy(renderer, texture, nullptr, &rect);
  
  int timeLineX = (int) ((textureW-1) * (pts - startTime) / duration);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);
  SDL_RenderDrawLine(renderer, timeLineX, rendererHeight - textureH - 2, timeLineX, rendererHeight);

}
