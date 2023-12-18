// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ui/VideoTextures.hh"
#include "ui/DisplayState.hh"

Resolution  vivictpp::ui::getNativeResolution(const vivictpp::ui::DisplayState &displayState) {
    Resolution leftDisplayResolution = displayState.leftVideoMetadata.filteredResolution.toDisplayResolution(
            displayState.leftVideoMetadata.filteredSampleAspectRatio);
    if (!displayState.rightVideoMetadata.empty()) {
        Resolution rightDisplayResolution = displayState.rightVideoMetadata.filteredResolution.toDisplayResolution(
                displayState.rightVideoMetadata.filteredSampleAspectRatio);
        return leftDisplayResolution.w > rightDisplayResolution.w ?
                leftDisplayResolution : rightDisplayResolution;
    } else {
        return leftDisplayResolution;
    }
}
