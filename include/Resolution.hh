// SPDX-FileCopyrightText: 2020 Sveriges Television AB
// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef RESOLUTION_HH
#define RESOLUTION_HH

struct Resolution {
  Resolution():
    Resolution(0,0) {};
  Resolution(int w, int h):
    w(w), h(h) {};
  Resolution(const Resolution &other) = default;
  int w;
  int h;
  float aspectRatio() const {
    return w / static_cast<float>(h);
  }
  Resolution scale(float scaleFactor) const {
    return Resolution(static_cast<int>(w * scaleFactor),
                      static_cast<int>(h * scaleFactor));
  }
};

#endif // RESOLUTION_HH
