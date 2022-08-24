// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef UI_VMAFGRAPH_HH
#define UI_VMAFGRAPH_HH

#include "vmaf/VmafLog.hh"
#include "logging/Logging.hh"

extern "C" {
#include <SDL.h>
#include <SDL_ttf.h>
}


namespace vivictpp {
namespace ui {

class VmafGraph {
public:
  VmafGraph(std::vector<vivictpp::vmaf::VmafLog> vmafLogs, float relativeWidth, float relativeHeight);

  void render(SDL_Renderer *renderer, double pts, double startTime, double duration);
  bool empty() { return _empty; };
private:
  void initTexture(SDL_Renderer *renderer);
    
private:
  std::vector<vivictpp::vmaf::VmafLog> vmafLogs;
  float relativeWidth;
  float relativeHeight;
  int rendererWidth;
  int rendererHeight;

  SDL_Texture *texture;
  int textureW = 0;
  int textureH = 0;
  vivictpp::logging::Logger logger;
  bool _empty;
};

}  // namespace vivictpp
}  // namespace ui

#endif // UI_VMAFGRAPH_HH
