// SPDX-FileCopyrightText: 2021 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include <cstdlib>
#include <iostream>

#include <sys/time.h>

int64_t currentTimeMillis() {
  struct timeval t;
  gettimeofday(&t, nullptr);
  return 1000 * t.tv_sec + t.tv_usec / 1000;
}

struct FloatRect {
  float x;
  float y;
  float w;
  float h;
};

struct IntRect {
  int x;
  int y;
  int w;
  int h;
};

std::uint8_t src[1920 * 1080];
int srcW = 1920;
int srcH = 1080;

std::uint8_t dst[1920 * 1080];
int dstStride = 1920;
IntRect dstRect = {0, 0, 1920, 1080};

void init() {
  for (int i = 0; i < srcW; i++) {
    for (int j = 0; j < srcH; j++) {
      src[i + j * srcW] = ((uint8_t)(rand() % 256));
    }
  }
}

// ~ 30ms
void bilinearProjectFloat() {

  int srcStride = srcW;
  FloatRect srcRect = {380.5, 270.2, 960.0, 539.2};

  float srcX, srcY;
  int x0, x1, y0, y1, x1Safe, y1Safe;
  int y0Offset, y1Offset;
  uint8_t v00, v01, v10, v11, v;
  for (int dy = 0; dy < dstRect.h; dy++) {
    srcY = srcRect.y + (srcRect.h - 1) * dy / dstRect.h;
    y0 = (int)srcY;
    y1 = (int)srcY + 1;
    y1Safe = y1 > srcH - 1 ? srcH - 1 : y1;
    y0Offset = y0 * srcStride;
    y1Offset = y1Safe * srcStride;
    for (int dx = 0; dx < dstRect.w; dx++) {
      srcX = srcRect.x + (srcRect.w - 1) * dx / dstRect.w;

      x0 = (int)srcX;
      x1 = (int)srcX + 1;
      x1Safe = x1 > srcW - 1 ? srcW - 1 : x1;
      v00 = src[x0 + y0Offset];
      v01 = src[x0 + y1Offset];
      v10 = src[x1Safe + y0Offset];
      v11 = src[x1Safe + y1Offset];
      v = (uint8_t)(v00 * (x1 - srcX) * (y1 - srcY) +
                    v10 * (srcX - x0) * (y1 - srcY) +
                    v01 * (x1 - srcX) * (srcY - y0) * v11 * (srcX - x0) *
                        (srcY - y0));
      dst[dx + dy * dstStride] = v;
    }
  }
}

void bilinearProjectFloat2() {

  int srcStride = srcW;
  FloatRect srcRect = {380.5, 270.2, 960.0, 539.2};

  float srcX, srcY;
  int x0, x1, y0, y1, x1Safe, y1Safe;
  int y0Offset, y1Offset;
  uint8_t v00, v01, v10, v11, v;

  float a_srcX[1920];
  int a_x0[1920];
  int a_x1[1920];
  int a_x1Safe[1920];
  float xd0[1920];
  float xd1[1920];
  for (int dx = 0; dx < dstRect.w; dx++) {
    a_srcX[dx] = srcRect.x + (srcRect.w - 1) * dx / dstRect.w;

    a_x0[dx] = (int)a_srcX[dx];
    a_x1[dx] = (int)a_srcX[dx] + 1;
    a_x1Safe[dx] = a_x1[dx] > srcW - 1 ? srcW - 1 : a_x1[dx];
    xd0[dx] = a_srcX[dx] - a_x0[dx];
    xd1[dx] = a_x1[dx] - a_srcX[dx];
  }
  float yd0, yd1;
  for (int dy = 0; dy < dstRect.h; dy++) {
    srcY = srcRect.y + (srcRect.h - 1) * dy / dstRect.h;
    y0 = (int)srcY;
    y1 = (int)srcY + 1;
    y1Safe = y1 > srcH - 1 ? srcH - 1 : y1;
    y0Offset = y0 * srcStride;
    y1Offset = y1Safe * srcStride;
    yd0 = srcY - y0;
    yd1 = y1 - srcY;
    for (int dx = 0; dx < dstRect.w; dx++) {
      srcX = a_srcX[dx];

      x0 = a_x0[dx];
      x1 = a_x1[dx];
      x1Safe = a_x1Safe[dx];
      v00 = src[x0 + y0Offset];
      v01 = src[x0 + y1Offset];
      v10 = src[x1Safe + y0Offset];
      v11 = src[x1Safe + y1Offset];
      v = (uint8_t)(v00 * xd1[dx] * yd1 + v10 * xd0[dx] * yd1 +
                    v01 * xd1[dx] * yd0 * v11 * xd0[dx] * yd0);
      dst[dx + dy * dstStride] = v;
    }
  }
}

// ~16ms
void bilinearProjectInt() {

  int srcStride = srcW;
  FloatRect srcRectFloat = {380.5, 270.2, 960.0, 539.2};
  IntRect srcRect = {(int)(380.5 * 65536), (int)(270.2 * 65536), 960 * 65536,
                     (int)(539.2 * 65536)};

  //  float srcX, srcY;
  int srcX, srcY;
  int x0, x1, y0, y1, x1Safe, y1Safe, x0Frac, x1Frac, y0Frac, y1Frac;
  int y0Offset, y1Offset;
  uint8_t v00, v01, v10, v11, v;
  for (int dy = 0; dy < dstRect.h; dy++) {
    srcY = srcRect.y + ((long)srcRect.h) * dy / dstRect.h;
    y0 = srcY >> 16;
    y1 = y0 + 1;
    y0Frac = y0 << 16;
    y1Frac = y1 << 16;
    y1Safe = y1 > srcH - 1 ? srcH - 1 : y1;
    y0Offset = y0 * srcStride;
    y1Offset = y1Safe * srcStride;
    for (int dx = 0; dx < dstRect.w; dx++) {
      srcX = srcRect.x + ((long)srcRect.w) * dx / dstRect.w;

      x0 = srcX >> 16;
      x1 = x0 + 1;
      x0Frac = x0 << 16;
      x1Frac = x1 << 16;
      x1Safe = x1 > srcW - 1 ? srcW - 1 : x1;
      v00 = src[x0 + y0Offset];
      v01 = src[x0 + y1Offset];
      v10 = src[x1Safe + y0Offset];
      v11 = src[x1Safe + y1Offset];
      v = (uint8_t)(((v00 * (x1Frac - srcX) * (y1Frac - srcY) +
                      v10 * (srcX - x0Frac) * (y1Frac - srcY) +
                      v01 * (x1Frac - srcX) * (srcY - y0Frac) +
                      v11 * (srcX - x0Frac) * (srcY - y0Frac))) >>
                    16);
      dst[dx + dy * dstStride] = v;
    }
  }
}

int srcX[1920];
int xi[1920];
int xd0[1920];
int xd1[1920];

void bilinearProjectInt2() {

  int srcStride = srcW;
  FloatRect srcRectFloat = {380.5, 270.2, 960.0, 539.2};
  IntRect srcRect = {(int)(380.5 * 65536), (int)(270.2 * 65536), 960 * 65536,
                     (int)(539.2 * 65536)};

  int dstStride = 1920;
  IntRect dstRect = {0, 0, 1920, 1080};

  int srcY;

  int xi0, xi1;
  int yi, yd0, yd1;
  int y0Offset, y1Offset;
  uint8_t v00, v01, v10, v11, v;

  for (int dx = 0; dx < dstRect.w; dx++) {
    srcX[dx] = srcRect.x + ((long)srcRect.w) * dx / dstRect.w;
    xi[dx] = srcX[dx] >> 16; // xIndex
    xd0[dx] = srcX[dx] - (xi[dx] << 16);
    xd1[dx] = dx == dstRect.w - 1 ? 0 : ((xi[dx] + 1) << 16) - srcX[dx];
  }
  for (int dy = 0; dy < dstRect.h; dy++) {
    srcY = srcRect.y + ((long)srcRect.h) * dy / dstRect.h;
    yi = srcY >> 16;
    yd0 = srcY - (yi << 16);
    yd1 = dy == dstRect.h - 1 ? 0 : ((yi + 1) << 16) - srcY;
    y0Offset = yi * srcStride;
    y1Offset = dy == dstRect.h ? y0Offset : (yi + 1) * srcStride;
    for (int dx = 0; dx < dstRect.w; dx++) {
      xi0 = xi[dx];
      xi1 = dx == dstRect.w - 1 ? xi0 : xi0 + 1;
      v00 = src[xi0 + y0Offset];
      v01 = src[xi0 + y1Offset];
      v10 = src[xi1 + y0Offset];
      v11 = src[xi1 + y1Offset];
      v = (uint8_t)(((v00 * xd1[dx] * yd1 + v10 * xd0[dx] * yd1 +
                      v01 * xd1[dx] * yd0 + v11 * xd0[dx] * yd0)) >>
                    16);
      dst[dx + dy * dstStride] = v;
    }
  }
}

void bilinearProjectInt3() {

  int srcStride = srcW;
  FloatRect srcRectFloat = {380.5, 270.2, 960.0, 539.2};
  IntRect srcRect = {(int)(380.5 * 65536), (int)(270.2 * 65536), 960 * 65536,
                     (int)(539.2 * 65536)};

  int dstStride = 1920;
  IntRect dstRect = {0, 0, 1920, 1080};

  int srcY;

  int xi0, xi1;
  int yi, yd0, yd1;
  int y0Offset, y1Offset;
  uint8_t v00, v01, v10, v11, v;

  long *ldst = (long *)dst;

  for (int dx = 0; dx < dstRect.w; dx++) {
    srcX[dx] = srcRect.x + ((long)srcRect.w) * dx / dstRect.w;
    xi[dx] = srcX[dx] >> 16; // xIndex
    xd0[dx] = srcX[dx] - (xi[dx] << 16);
    xd1[dx] = dx == dstRect.w - 1 ? 0 : ((xi[dx] + 1) << 16) - srcX[dx];
  }
  long lv;
  for (int dy = 0; dy < dstRect.h; dy++) {
    srcY = srcRect.y + ((long)srcRect.h) * dy / dstRect.h;
    yi = srcY >> 16;
    yd0 = srcY - (yi << 16);
    yd1 = dy == dstRect.h - 1 ? 0 : ((yi + 1) << 16) - srcY;
    y0Offset = yi * srcStride;
    y1Offset = dy == dstRect.h ? y0Offset : (yi + 1) * srcStride;
    for (int i = 0; i < dstRect.w / 8; i++) {
      lv = 0;
      for (int j = 0; j < 8; j++) {
        int dx = 8 * i + j;
        xi0 = xi[dx];
        xi1 = dx == dstRect.w - 1 ? xi0 : xi0 + 1;
        v00 = src[xi0 + y0Offset];
        v01 = src[xi0 + y1Offset];
        v10 = src[xi1 + y0Offset];
        v11 = src[xi1 + y1Offset];
        lv += 0xFF & (((v00 * xd1[dx] * yd1 + v10 * xd0[dx] * yd1 +
                        v01 * xd1[dx] * yd0 + v11 * xd0[dx] * yd0)) >>
                      16);
        lv << 8;
      }
      ldst[i + dy * dstStride / 8] = lv;
    }
  }
}

void sampleProjectInt() {

  int srcStride = srcW;
  FloatRect srcRectFloat = {380.5, 270.2, 960.0, 539.2};
  IntRect srcRect = {(int)(380.5 * 65536), (int)(270.2 * 65536), 960 * 65536,
                     (int)(539.2 * 65536)};

  int dstStride = 1920;
  IntRect dstRect = {0, 0, 1920, 1080};

  int srcY;

  int xi[1920];

  long *ldst = (long *)dst;

  for (int dx = 0; dx < dstRect.w; dx++) {
    srcX[dx] = srcRect.x + ((long)srcRect.w) * dx / dstRect.w;
    xi[dx] = srcX[dx] >> 16; // xIndex
  }
  for (int dy = 0; dy < dstRect.h; dy++) {
    srcY = srcRect.y + ((long)srcRect.h) * dy / dstRect.h;
    int yi = srcY >> 16;
    int yOffset = yi * srcStride;
    for (int dx = 0; dx < dstRect.w; dx++) {
      dst[dx + dy * dstStride] = src[xi[dx] + yOffset];
    }
  }
}

void time(std::string name, int n, void (*f)()) {
  int64_t t = 0;
  for (int i = 0; i < n; i++) {
    int64_t t0 = currentTimeMillis();
    f();
    t += currentTimeMillis() - t0;
    //    std::cout << (int) dst[i] << " "; // << rand() % 256 << " ";
  }

  std::cout << name << "\n" << (t / n) << "ms\n";
}

int main(int argc, char **argv) {
  init();
  time("bilinearProjectInt3", 100, &bilinearProjectInt2);
  time("bilinearProjectInt2", 100, &bilinearProjectInt2);
  time("bilinearProjectInt", 100, &bilinearProjectInt);
  time("bilinearProjectFloat", 100, &bilinearProjectFloat);
  time("bilinearProjectFloat2", 100, &bilinearProjectFloat2);
  time("sampleProjectInt", 100, &sampleProjectInt);

  /*
    int64_t t0 = currentTimeMillis();
    int n = 100;
    for (int i =0; i < n; i++) {
    bilinearProjectInt2();
    }
    int64_t t = currentTimeMillis() - t0;
    std::cout << (t / n) << "ms\n";
  */
}
