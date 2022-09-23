// SPDX-FileCopyrightText: 2020 Sveriges Television AB
// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef UI_FONTSIZE_HH
#define UI_FONTSIZE_HH

extern "C" {
#include <SDL.h>
}

#include <exception>
#include <cmath>

namespace vivictpp::ui {

const float REFERENCE_DPI = 170;

class FontSize {
public:
    FontSize(int size):
        size(size) {};

    static void setScaling(bool dpiScaling, float customScaleFactor) {
        float dpiScaleFactor = 1.0;
        if (dpiScaling) {
            float dpi;
            SDL_GetDisplayDPI(0, &dpi, nullptr, nullptr);
            dpiScaleFactor = dpi / REFERENCE_DPI;
        }
        scaleFactor = dpiScaleFactor * customScaleFactor;
        scalingEnabled = true;
    }

    operator int() const { return scaledSize(); };

    int scaledSize() const { return (int) round(size * (scalingEnabled ? scaleFactor : 1.0)); };
private:
    int size;
    static bool scalingEnabled;
    static float scaleFactor;
};

}; // vivictpp::ui;

#endif // UI_FONTSIZE_HH
