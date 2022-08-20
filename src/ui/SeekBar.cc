// SPDX-FileCopyrightText: 2022 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ui/SeekBar.hh"

extern "C" {
#include <SDL.h>
}


void vivictpp::ui::SeekBar::render(SDL_Renderer *renderer, int x, int y) {
    (void) y;
    int width, height;
    SDL_GetRendererOutputSize(renderer, &width, &height);
    int x0 = x + margin.left;
    int y0 = y + margin.top;
    int w = width - margin.left - margin.right;
    int h = seekBarHeight;
    SDL_SetRenderDrawColor(renderer, 125, 125, 125, state.opacity);
    SDL_Rect seekBarRect {x0, y0 + 2, w, h - 4};
    SDL_RenderFillRect(renderer, &seekBarRect);
    SDL_SetRenderDrawColor(renderer, 125, 255, 125, state.opacity);
    float rp = state.seeking ? state.relativeSeekPos : state.relativePos;
    seekBarRect.w = (int) (seekBarRect.w * rp);
    SDL_RenderFillRect(renderer, &seekBarRect);
    seekBarRect.x = margin.left + seekBarRect.w - (seekBarHeight + 2)/2;
    seekBarRect.y = y0 - 3;
    seekBarRect.w = seekBarHeight + 2;
    seekBarRect.h = seekBarHeight + 6;
    if (state.seeking) {
      SDL_SetRenderDrawColor(renderer, 200, 200, 200, state.opacity);
    } else {
      SDL_SetRenderDrawColor(renderer, 255, 255, 255, state.opacity);
    }
    SDL_RenderFillRect(renderer, &seekBarRect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, state.opacity);
    SDL_RenderDrawRect(renderer, &seekBarRect);
    box = {x0, y0, w, h};
}
