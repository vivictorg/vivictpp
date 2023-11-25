// SPDX-FileCopyrightText: 2020 Sveriges Television AB
// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef UI_DISPLAYSTATE_HH
#define UI_DISPLAYSTATE_HH

#include <string>
#include <cmath>
#include <iostream>
#include <array>
#include <vector>

#include "VideoMetadata.hh"
#include "time/Time.hh"
#include "libav/Frame.hh"
#include "libav/DecoderMetadata.hh"


namespace vivictpp {
namespace ui {

class Zoom {
 private:
  int zoom_ = 0;
  float multiplier_ = 1.0;
 public:
  void set(int zoom) {
    zoom_ = zoom;
    setMultiplier();
  }
  int get() const { return zoom_; }
  int increment() {
    zoom_++;
    setMultiplier();
    return zoom_;
  }
  int decrement() {
    zoom_--;
    setMultiplier();
    return zoom_;
  }
  float multiplier() const { return multiplier_; }
 private:
  void setMultiplier() { multiplier_ = pow(1.2, zoom_); }
};

struct SeekBarState {
  bool visible{false};
  int64_t hideTimer{0};
  int opacity{255};
  float relativePos{0};
  float relativeSeekPos{0};
  bool seeking{false};
};

struct DisplayState {
  float splitPercent{50};
  //  int zoom{0};
  Zoom zoom;
  float panX{0};
  float panY{0};
  bool fullscreen{false};
  std::string timeStr;
  bool displayTime{true};
  bool displayMetadata{true};
  bool displayPlot{true};
  bool splitScreenDisabled{false};
  bool fitToScreen{true};
  bool isPlaying{false};
  vivictpp::time::Time pts{0};
  SeekBarState seekBar;
  int leftFrameOffset{0};
  vivictpp::libav::Frame leftFrame;
  vivictpp::libav::Frame rightFrame;
  VideoMetadata leftVideoMetadata;
  VideoMetadata rightVideoMetadata;
  libav::DecoderMetadata leftDecoderMetadata;
  libav::DecoderMetadata rightDecoderMetadata;
  int videoMetadataVersion{0};
  std::string playbackSpeedStr;
  bool displayImGuiDemo{false};
  bool displayHelp{false};
  bool displayAbout{false};
  bool displaySettingsDialog{false};
  bool displayLogs{false};

  void updateFrames(std::array<vivictpp::libav::Frame, 2> frames) {
    leftFrame = frames[0];
    rightFrame = frames[1];
  };

  void updateMetadata(std::array<std::vector<VideoMetadata>, 2> metadata) {
    leftVideoMetadata = metadata[0][0];
    if (!metadata[1].empty()) {
      rightVideoMetadata = metadata[1][0];
    }
    videoMetadataVersion++;
  }

  void updateDecoderMetadata(std::array<vivictpp::libav::DecoderMetadata, 2> metadata) {
    leftDecoderMetadata = metadata[0];
    rightDecoderMetadata = metadata[1];
  }
};

}  // ui
}  // vivictpp

#endif // UI_DISPLAYSTATE_HH
