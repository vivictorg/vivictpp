// SPDX-FileCopyrightText: 2023 Gustav Grusell
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIVICTPP_UI_SDLVIDEOTEXTURES_HH
#define VIVICTPP_UI_SDLVIDEOTEXTURES_HH


#include "VideoTextures.hh"
#include "sdl/SDLUtils.hh"
#include "DisplayState.hh"

namespace vivictpp::imgui {
    class ImGuiSDL;
}

namespace vivictpp::ui {

    class SDLVideoTextures {
        friend class vivictpp::imgui::ImGuiSDL;
    public:
        bool update(struct SDL_Renderer *renderer, const DisplayState &displayState);
    private:
        bool initTextures(struct SDL_Renderer *renderer, const DisplayState &displayState);
    private:
        vivictpp::sdl::SDLTexture leftTexture;
        vivictpp::sdl::SDLTexture rightTexture;
        vivictpp::ui::VideoTextures videoTextures;
        int videoMetadataVersion{-1};
    };

}  // namespace vivictpp::ui
#endif //VIVICTPP_UI_SDLVIDEOTEXTURES_HH
