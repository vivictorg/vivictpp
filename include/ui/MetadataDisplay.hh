// SPDX-FileCopyrightText: 2022 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef UI_METADATADISPLAY_HH
#define UI_METADATADISPLAY_HH

#include "VideoMetadata.hh"
#include "ui/DisplayState.hh"
#include "ui/FrameMetadataBox.hh"
#include "ui/Ui.hh"
#include "ui/Container.hh"

#include <functional>

namespace vivictpp::ui {

typedef std::function<const VideoMetadata&(const DisplayState&)> VideoMetadataAccessor;
typedef std::function<FrameMetadata(const DisplayState&)> FrameMetadataAccessor;

class MetadataDisplay : public Component {
public:
  MetadataDisplay(Position position, bool showFrameOffset,
                  VideoMetadataAccessor videoMetadataAccessor, FrameMetadataAccessor frameMetadataAccessor);
  virtual ~MetadataDisplay() = default;
  void render(const DisplayState &displayState, SDL_Renderer *renderer, int x, int y) override {
    container.render(displayState, renderer, x, y);
  };
  const Box& getBox() const override {
    return container.getBox();
  }
  void update(const DisplayState &displayState);
private:
  std::shared_ptr<TextBox> streamMetadata;
  std::shared_ptr<FrameMetadataBox> frameMetadataBox;
  std::shared_ptr<TextBox> offsetBox;
  FixedPositionContainer container;
  bool showFrameOffset;
  VideoMetadataAccessor videoMetadataAccessor;
  FrameMetadataAccessor frameMetadataAccessor;
  int videoMetadataVersion{-1};
};


}; // namespace vivictpp::ui

#endif // UI_METADATADISPLAY_HH
