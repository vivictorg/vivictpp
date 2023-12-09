// SPDX-FileCopyrightText: 2020 Sveriges Television AB
// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef RESOLUTION_HH
#define RESOLUTION_HH

#include <libavutil/rational.h>
#include <sstream>
#include <string>

struct Resolution {
  Resolution() : Resolution(0, 0){};
  Resolution(int w, int h) : w(w), h(h){};
  //  Resolution(const Resolution &other) = default;
  int w;
  int h;
  float aspectRatio() const { return w / static_cast<float>(h); }
  Resolution scale(float scaleFactor) const {
    return Resolution(static_cast<int>(w * scaleFactor),
                      static_cast<int>(h * scaleFactor));
  }
  Resolution toDisplayResolution(const AVRational sampleAspectRatio) const {
    return Resolution(w * sampleAspectRatio.num / sampleAspectRatio.den, h);
  }
  Resolution scaleToWidth(const int width) const {
    return Resolution(width, h * width / w);
  }
  Resolution scaleKeepingAspectRatio(int maxw, int maxh) const {
    if (w * maxh < h * maxw) {
      return Resolution(w * maxh / h, maxh);
    } else {
      return Resolution(maxw, h * maxw / w);
    }
  }
  Resolution padToAspectRatio(const AVRational aspectRatio) const {
    if (w * aspectRatio.den == h * aspectRatio.num) {
      return Resolution(w, h);
    }
    if (w * aspectRatio.num > h * aspectRatio.den) {
      // is wider than the aspect ratio, pad height
      return Resolution(w, w * aspectRatio.den / aspectRatio.num);
    }
    // is narrower than the aspect ratio, pad width
    return Resolution(h * aspectRatio.num / aspectRatio.den, h);
  }

  std::string toString() const {
    std::ostringstream oss;
    oss << this->w << "x" << this->h;
    return oss.str();
  }
};

bool operator==(const Resolution &r1, const Resolution &r2);
bool operator!=(const Resolution &r1, const Resolution &r2);

#endif // RESOLUTION_HH
