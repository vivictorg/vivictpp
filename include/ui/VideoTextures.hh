#ifndef VIVICTPP_UI_VIDEOTEXTURES_HH_
#define VIVICTPP_UI_VIDEOTEXTURES_HH_

#include "Resolution.hh"
#include "sdl/SDLUtils.hh"
#include "ui/DisplayState.hh"

namespace vivictpp::ui {

  class VideoTextures {
  public:
    vivictpp::sdl::SDLTexture leftTexture;
    vivictpp::sdl::SDLTexture rightTexture;
    Resolution nativeResolution;
  public:
    bool update(SDL_Renderer *renderer, const DisplayState &displayState);
  private:
    bool initTextures(SDL_Renderer *renderer, const DisplayState &displayState);
    int videoMetadataVersion{-1};
  };
}  // namespace vivictpp::ui


#endif /* VIVICTPP_UI_VIDEOTEXTURES_HH_ */
