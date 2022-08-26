// SPDX-FileCopyrightText: 2022 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ui/MetadataDisplay.hh"
#include "SDL_render.h"
#include "VideoMetadata.hh"
#include "ui/Container.hh"
#include "ui/FrameMetadataBox.hh"

vivictpp::ui::MetadataDisplay::MetadataDisplay(Position position, bool showFrameOffset,
                                               VideoMetadataAccessor videoMetadataAccessor,
                                               FrameMetadataAccessor frameMetadataAccessor):
  streamMetadata(std::make_shared<TextBox>("", "FreeMono", 16, "Stream Info")),
  frameMetadataBox(std::make_shared<FrameMetadataBox>("FreeMono", 16)),
  offsetBox(std::make_shared<TextBox>("", "FreeMono", 16, "Frame offset", 120)),
  container(position, { streamMetadata, frameMetadataBox, offsetBox}),
  showFrameOffset(showFrameOffset),
  videoMetadataAccessor(videoMetadataAccessor),
  frameMetadataAccessor(frameMetadataAccessor) {
  streamMetadata->bg = {50, 50, 50, 100};
  frameMetadataBox->bg = {50, 50, 50, 100};
  offsetBox->bg = {50, 50, 50, 100};
}

void vivictpp::ui::MetadataDisplay::update(const DisplayState &displayState) {
  if (displayState.videoMetadataVersion != videoMetadataVersion) {
    const VideoMetadata &videoMetadata = videoMetadataAccessor(displayState);
    streamMetadata->setText(videoMetadata.toString());
    videoMetadataVersion = displayState.videoMetadataVersion;
  }
  streamMetadata->display = displayState.displayMetadata;
  frameMetadataBox->display = displayState.displayMetadata && !displayState.isPlaying;
  offsetBox->display = showFrameOffset && displayState.displayMetadata && displayState.leftFrameOffset != 0;
  if (frameMetadataBox->display) {
    FrameMetadata frameMetadata = frameMetadataAccessor(displayState);
    frameMetadataBox->update(frameMetadata);
  }
  if (offsetBox->display) {
    offsetBox->setText(fmt::format(" {}", displayState.leftFrameOffset));
  }
}



