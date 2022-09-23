// SPDX-FileCopyrightText: 2022 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef UI_FRAMEMETADATA_HH
#define UI_FRAMEMETADATA_HH

#include "ui/TextBox.hh"
#include "VideoMetadata.hh"

#include "fmt/core.h"

namespace vivictpp::ui {

class FrameMetadataBox: public vivictpp::ui::TextBox {
public:
  FrameMetadataBox(std::string font, int fontSize, int minWidth = 0, int minHeight = 0,
                Margin margin = {2,2,2,2}):
    TextBox("", font, fontSize, "Frame Info", minWidth, minHeight, margin) {}
  void update(const FrameMetadata &metadata) {
    setText(fmt::format("Frametype: {}\nFrame size: {}\n",
                        metadata.pictureType, metadata.size));
  };
};

}; // namespace vivictpp::ui

#endif // UI_FRAMEMETADATA_HH
