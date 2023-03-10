#ifndef VIVICTPP_UI_VIDEOSCALER_HH_
#define VIVICTPP_UI_VIDEOSCALER_HH_

#include "Resolution.hh"
extern "C" {
#include <SDL.h>
#include <SDL_thread.h>
}

#include "DisplayState.hh"


namespace vivictpp::ui {

  typedef SDL_Rect Rectangle;

  /**
     Class for calculate source and destination rectangles for videos,
     taking into account zoom, pan, keeping aspect ratio etc.

   */
  
  class VideoScaler {
  public:
    Rectangle sourceRectLeft;
    Rectangle sourceRectRight;
    Rectangle destRectLeft;
    Rectangle destRectRight;
    Rectangle destRect;
    // Default resolution to render, ie largest resolution of source videos
    Resolution defaultResolution;
  public:
    void update(const DisplayState &displayState, Resolution displaySize);
  private:
    void setDefaultSourceRectangles(const DisplayState &displayState);
    void calcZoomedSrcRect(const DisplayState &displayState,
                           const Resolution &displaySize,
                           const Resolution &scaledResolution,
                           const VideoMetadata &videoMetadata,
                           SDL_Rect &rect);
  };

}  // namespace vivictpp::ui

#endif /* VIVICTPP_UI_VIDEOSCALER_HH_ */
