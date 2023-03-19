#ifndef VIVICTPP_IMGUI_EVENTS_HH_
#define VIVICTPP_IMGUI_EVENTS_HH_

#include "time/Time.hh"

#include <string>
#include <memory>

namespace vivictpp::imgui {

class Event {
public:
  virtual ~Event() = default;
};

class Quit: public Event {};

class WindowSizeChange: public Event {};

class MouseMotion: public Event {
public:
  MouseMotion(int x, int y):
    x(x), y(y) { }
  int x;
  int y;
};

class KeyEvent: public Event {
public:
  KeyEvent(std::string keyName, bool shift, bool ctrl, bool alt):
    keyName(keyName),
    shift(shift),
    ctrl(ctrl),
    alt(alt) {};
  std::string keyName;
  bool shift{false};
  bool ctrl{false};
  bool alt{false};
};


enum ActionType {
  NoAction,
  ActionQuit,
  PlayPause,
  ZoomIn,
  ZoomOut,
  ZoomReset,
  Seek,
  SeekRelative,
  StepForward,
  StepBackward,
  FrameOffsetIncrease,
  FrameOffsetDecrease,
  ToggleFullscreen,
  ToggleDisplayTime,
  ToggleDisplayMetadata,
  ToggleDisplayPlot,
  ToggleFitToScreen,
  PlaybackSpeedIncrease,
  PlaybackSpeedDecrease
};

struct Action {
  Action(ActionType type, vivictpp::time::Time seek = 0):
    type(type),
    seek(seek){};
  ActionType type;
  vivictpp::time::Time seek;
};

}  // namespace vivictpp::imgui


#endif /* VIVICTPP_IMGUI_EVENTS_HH_ */
