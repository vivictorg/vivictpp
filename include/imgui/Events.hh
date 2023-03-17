#ifndef VIVICTPP_IMGUI_EVENTS_HH_
#define VIVICTPP_IMGUI_EVENTS_HH_

#include "time/Time.hh"

namespace vivictpp::imgui {

enum EventType {Quit, MouseMotion, WindowSizeChange, PlayPause, ZoomIn, ZoomOut, ZoomReset, Seek, ToggleFullscreen};

struct MouseMotion {
  int x;
  int y;
};


struct Event {
  EventType type;
  union {
    vivictpp::time::Time seek{0};
    struct MouseMotion mouseMotion;
  };

};


}  // namespace vivictpp::imgui


#endif /* VIVICTPP_IMGUI_EVENTS_HH_ */
