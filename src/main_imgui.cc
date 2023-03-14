// Dear ImGui: standalone example application for SDL2 + SDL_Renderer
// (SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

// Important to understand: SDL_Renderer is an _optional_ component of SDL. We do not recommend you use SDL_Renderer
// because it provides a rather limited API to the end-user. We provide this backend for the sake of completeness.
// For a multi-platform app consider using e.g. SDL+DirectX on Windows and SDL+OpenGL on Linux/OSX.


#include "SDL_render.h"
#include "SDL_video.h"
#include "VideoPlayback.hh"
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"
#include <stdio.h>
#include <SDL.h>

#include "OptParser.hh"
#include "VivictPP.hh"
#include "Controller.hh"
#include "SourceConfig.hh"
#include "time/TimeUtils.hh"
#include "ui/DisplayState.hh"
#include "sdl/SDLUtils.hh"
#include <vector>
#include <algorithm>


#if !SDL_VERSION_ATLEAST(2,0,17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

// https://github.com/ocornut/imgui/issues/3379`
bool ScrollWhenDraggingOnVoid(const ImVec2& delta, ImGuiMouseButton mouse_button)
{
  // blow
    ImGuiContext& g = *ImGui::GetCurrentContext();
    ImGuiWindow* window = g.CurrentWindow;
    ImGuiID id = window->GetID("##scrolldraggingoverlay");
    ImGui::KeepAliveID(id);
    bool hovered = false;
    bool held = false;
    bool scrolled = false;
    ImGuiButtonFlags button_flags = (mouse_button == 0) ? ImGuiButtonFlags_MouseButtonLeft : (mouse_button == 1) ? ImGuiButtonFlags_MouseButtonRight : ImGuiButtonFlags_MouseButtonMiddle;
    if (g.HoveredId == 0) // If nothing hovered so far in the frame (not same as IsAnyItemHovered()!)
        ImGui::ButtonBehavior(window->Rect(), id, &hovered, &held, button_flags);
    if (held && delta.x != 0.0f) {
        ImGui::SetScrollX(window, window->Scroll.x - delta.x);
        scrolled = true;
    }
    if (held && delta.y != 0.0f) {
        ImGui::SetScrollY(window, window->Scroll.y - delta.y);
        scrolled = true;
    }
    return scrolled;
}

void logVec(const std::string &str, ImVec2 vec) {
  spdlog::info("{}: {}x{}", str, vec.x, vec.y);
}

// Main code
int main(int argc, char** argv)
{
  vivictpp::OptParser optParser;
  if (!optParser.parseOptions(argc, argv)) {
    return optParser.exitCode;
  }
  VivictPPConfig vivictPPConfig = optParser.vivictPPConfig;

    // Setup SDL
    // (Some versions of SDL before <2.0.10 appears to have performance/stalling issues on a minority of Windows systems,
    // depending on whether SDL_INIT_GAMECONTROLLER is enabled or disabled.. updating to the latest version of SDL is recommended!)
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // Setup window
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("Dear ImGui SDL2+SDL_Renderer example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);

    int windowWidth;
    int windowHeight;
    SDL_GetWindowSize(window,&windowWidth, &windowHeight);

    // Setup SDL_Renderer instance
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if (renderer == NULL)
    {
        SDL_Log("Error creating SDL_Renderer!");
        return 0;
    }
    //SDL_RendererInfo info;
    //SDL_GetRendererInfo(renderer, &info);
    //SDL_Log("Current SDL_Renderer: %s", info.name);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    (void)io;
//    io.DeltaTime = 1.0f/24.0f;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer_Init(renderer);

    // Our state
    ImVec4 clear_color = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);

    // Main loop
    bool done = false;
    bool fullscreen = false;

//    VideoInputs videoInputs(vivictPPConfig);
    vivictpp::VideoPlayback videoPlayback(vivictPPConfig);

    vivictpp::ui::DisplayState displayState;
    displayState.splitScreenDisabled = vivictPPConfig.sourceConfigs.size() == 1;
    if (displayState.splitScreenDisabled) {
      displayState.splitPercent = 100;
    }
    displayState.updateFrames(videoPlayback.getVideoInputs().firstFrames());
    displayState.updateMetadata(videoPlayback.getVideoInputs().metadata());


    vivictpp::ui::VideoTextures videoTextures;
    videoTextures.update(renderer, displayState);

    SDL_SetWindowSize(window, videoTextures.nativeResolution.w, videoTextures.nativeResolution.h);

    int64_t tLastPresent = 0;

    struct {
      ImVec2 pos;
      ImVec2 scroll;
      ImVec2 size;
      ImVec2 videoSize;
      ImVec2 videoPos;
      bool scrollUpdated{false};
    } videoWindow;

    float seekValue = 0.0f;
//    float lastSeekValue = 0.0f;
    int showControls = 0;

    while (!done)
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
              windowWidth = event.window.data1;
              windowHeight = event.window.data2;
            }
            if (event.type == SDL_MOUSEMOTION && !displayState.splitScreenDisabled) {
              SDL_MouseMotionEvent mouseEvent = event.motion;
              displayState.splitPercent = 100.0 * std::clamp((mouseEvent.x - videoWindow.videoPos.x) / windowWidth, 0.0f, 1.0f);
            }
        }

        // Start the Dear ImGui frame

        ImGui_ImplSDLRenderer_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        {
          ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration |
                                          ImGuiWindowFlags_NoTitleBar |
                                              ImGuiWindowFlags_AlwaysAutoResize |
                                              ImGuiWindowFlags_NoSavedSettings |
                                              ImGuiWindowFlags_NoFocusOnAppearing |
                                              ImGuiWindowFlags_NoNav;
          ImVec2 windowSize = ImGui::GetWindowSize();
          const float PAD = 10.0f;
          const ImGuiViewport* viewport = ImGui::GetMainViewport();
          ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
          ImVec2 work_size = viewport->WorkSize;

          videoWindow.pos = {0,0};
          videoWindow.size = work_size;
          ImGui::SetNextWindowPos(videoWindow.pos);
          ImGui::SetNextWindowSize(videoWindow.size);
          //         int scaleFactor = 2;
          float scaleFactor = displayState.zoom.multiplier();
//          ImVec2 videoSize(1920, 800);
          ImVec2 scaledVideoSize = {videoTextures.nativeResolution.w * displayState.zoom.multiplier() , videoTextures.nativeResolution.h * scaleFactor};
          videoWindow.videoSize = scaledVideoSize;
          ImGui::SetNextWindowContentSize({std::max(work_size.x, scaledVideoSize.x), std::max(work_size.y, scaledVideoSize.y)});
          ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
          ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

          bool myBool2;
          if (ImGui::Begin("Vivict++", &myBool2,  ImGuiWindowFlags_AlwaysAutoResize |
                           ImGuiWindowFlags_NoDecoration |
                           ImGuiWindowFlags_NoSavedSettings |
                           ImGuiWindowFlags_NoFocusOnAppearing |
                           ImGuiWindowFlags_NoNav |
                           ImGuiWindowFlags_NoBringToFrontOnFocus |
                           ImGuiWindowFlags_NoTitleBar |
                           ImGuiWindowFlags_NoScrollbar
                )) {

            int64_t tNextPresent = tLastPresent + (int64_t) (1e6 * io.DeltaTime);
            if (videoPlayback.checkdvanceFrame(tNextPresent)) {
              displayState.updateFrames(videoPlayback.getVideoInputs().firstFrames());
            }
            int w,h;
            SDL_GetRendererOutputSize(renderer, &w, &h);

            ImVec2 viewSize(w, h);

            if (videoWindow.scrollUpdated) {
              ImGui::SetScrollX(videoWindow.scroll.x);
              ImGui::SetScrollY(videoWindow.scroll.y);
              videoWindow.scrollUpdated = false;
            } else {
              videoWindow.scroll = {ImGui::GetScrollX(), ImGui::GetScrollY()};
            }
            float scrollX = videoWindow.scroll.x;
            float scrollY = videoWindow.scroll.y;

            ImVec2 pad = ImGui::GetCursorPos();
            if (scaledVideoSize.x < viewSize.x) {
              pad.x += (viewSize.x - scaledVideoSize.x) / 2;
            }
            if (scaledVideoSize.y < viewSize.y) {
              pad.y += (viewSize.y - scaledVideoSize.y) / 2;
            }
            videoWindow.videoPos = pad;

            videoTextures.update(renderer, displayState);

            float splitFraction = displayState.splitPercent / 100;
            float splitX = pad.x + splitFraction * viewSize.x;
            ImVec2 drawPos = {pad.x - scrollX, pad.y - scrollY}; //cursorPos;
            ImVec2 uvMin(0, 0);

            ImVec2 uvMax((scrollX + splitX - pad.x) / scaledVideoSize.x ,1);
            ImVec2 p2(pad.x + scaledVideoSize.x - scrollX, pad.y + scaledVideoSize.y - scrollY);
            ImGui::GetWindowDrawList()->AddImage((void*)(intptr_t) videoTextures.leftTexture.get(),
                                                 drawPos, p2 /*, uvMin, uvMax*/);

            if (!displayState.splitScreenDisabled) {
              // TODO: Should take into account size of video texture here
              drawPos.x = splitX;
              uvMin.x = uvMax.x;
              uvMax.x = 1.0; //(scrollX + viewSize.x) / scaledVideoSize.x;
              ImGui::GetWindowDrawList()->AddImage((void*)(intptr_t) videoTextures.rightTexture.get(),
                                                 drawPos, p2, uvMin, uvMax);
              ImGui::GetWindowDrawList()->AddLine({splitX, pad.y}, {splitX, pad.y + scaledVideoSize.y}, 0x80FFFFFF, 0.5);
            }
            /*
            if (ImGui::InvisibleButton("##playpause", videoWindow.size)) {
                videoPlayback.togglePlaying();
            };
            */
            ImVec2 mouse_delta = ImGui::GetIO().MouseDelta;
            if (ScrollWhenDraggingOnVoid(mouse_delta, 0)) {
              videoWindow.scroll = {ImGui::GetScrollX(), ImGui::GetScrollY()};
            }

          }
          ImGui::End();
          ImGui::PopStyleVar(2);

          ImVec2 window_pos, window_pos_pivot;
          window_pos.x = work_pos.x + work_size.x / 2;
          window_pos.y = work_size.y - PAD;
          window_pos_pivot.x = 0.5f;
          window_pos_pivot.y = 1.0f;
          ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
          ImGui::SetNextWindowSize({work_size.x - 60, 60});
          window_flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground;
          ImGui::SetNextWindowBgAlpha(0.10f); // Transparent background
          bool myBool;

          if (ImGui::Begin("Video controls", &myBool, window_flags)) {
            if (ImGui::IsWindowHovered()) {
              showControls = 70;
            } else if (showControls > 0) {
              showControls--;
            }
            if (showControls > 0) {
              ImGui::PushStyleVar(ImGuiStyleVar_Alpha, std::clamp(showControls / 60.0f, 0.0f, 1.0f));
              if (ImGui::Button(videoPlayback.isPlaying() ? "Pause" : "Play")) {
                videoPlayback.togglePlaying();
              }
              ImGui::SameLine();
              if (ImGui::Button("Step")) {
                /*
                if (videoInputs.ptsInRange(videoInputs.nextPts())) {
                  pts = videoInputs.nextPts();
                  //                  spdlog::info("Stepping to {}", pts);
                  videoInputs.stepForward(pts);
                }
                */
              }
              ImGui::SameLine();
              if (ImGui::Button("Fullscreen")) {
                fullscreen = !fullscreen;
                if (fullscreen) {
                  SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                } else {
                  SDL_SetWindowFullscreen(window, 0);
                }
              }
              ImGui::SameLine();
              if (ImGui::Button("Zoom in")) {
                ImVec2 oldVideoSize = videoWindow.videoSize;
                displayState.zoom.increment();
                videoWindow.videoSize = {videoTextures.nativeResolution.w * displayState.zoom.multiplier(),
                  videoTextures.nativeResolution.h * displayState.zoom.multiplier()};
                if (videoWindow.videoSize.x > videoWindow.size.x ||
                    videoWindow.videoSize.y > videoWindow.size.y) {
                  // Adjust scroll so center of picture stays the same
                  // TODO: Take 'pad' into account maybe
                  ImVec2 center = { (videoWindow.scroll.x + videoWindow.size.x / 2) / oldVideoSize.x,
                    (videoWindow.scroll.y + videoWindow.size.y / 2) / oldVideoSize.y };
                  videoWindow.scroll = { center.x * videoWindow.videoSize.x - videoWindow.size.x / 2,
                    center.y * videoWindow.videoSize.y - videoWindow.size.y / 2 };
                  videoWindow.scrollUpdated = true;
                } else {
                  if (videoWindow.scroll.x != 0.0 || videoWindow.scroll.y != 0.0) {
                    videoWindow.scroll = { 0, 0};
                    videoWindow.scrollUpdated = true;
                  }
                }
              }
              ImGui::SameLine();
              if (ImGui::Button("Zoom out")) {
                ImVec2 oldVideoSize = videoWindow.videoSize;
                displayState.zoom.decrement();
                videoWindow.videoSize = {videoTextures.nativeResolution.w * displayState.zoom.multiplier(),
                  videoTextures.nativeResolution.h * displayState.zoom.multiplier()};
                if (videoWindow.videoSize.x > videoWindow.size.x ||
                    videoWindow.videoSize.y > videoWindow.size.y) {
                  // Adjust scroll so center of picture stays the same
                  ImVec2 center = { (videoWindow.scroll.x + videoWindow.size.x / 2) / oldVideoSize.x,
                    (videoWindow.scroll.y + videoWindow.size.y / 2) / oldVideoSize.y };
                  videoWindow.scroll = { center.x * videoWindow.videoSize.x - videoWindow.size.x / 2,
                    center.y * videoWindow.videoSize.y - videoWindow.size.y / 2 };
                  videoWindow.scrollUpdated = true;

                } else {
                  if (videoWindow.scroll.x != 0.0 || videoWindow.scroll.y != 0.0) {
                    videoWindow.scroll = { 0, 0};
                    videoWindow.scrollUpdated = true;
                  }
                }
              }
              ImGui::SameLine();
              if (ImGui::Button("Reset zoom")) {
                displayState.zoom.set(0);
              }
              ImGui::PushItemWidth(videoWindow.size.x - 60);
              ImGui::SliderFloat("##Seekbar", &seekValue, 0.0f,  videoPlayback.getVideoInputs().duration() / 1e6);
              ImGui::PopItemWidth();
              if (ImGui::IsItemActive()) {
                showControls = 70;
              }
              if (ImGui::IsItemActivated()) {
                spdlog::info("Drag start");
              }

              if (ImGui::IsItemDeactivatedAfterEdit()) {
                spdlog::info("Drag end: {}", seekValue);
                vivictpp::time::Time seekPos = (uint64_t) 1e6 * seekValue;
                videoPlayback.seek(seekPos);
              }
              ImGui::PopStyleVar();
            }
          }
          ImGui::End();
        }


        // Rendering
        ImGui::Render();
        SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());

//        spdlog::info("io.deltaTime: {}", io.DeltaTime);

        SDL_RenderPresent(renderer);
        tLastPresent = vivictpp::time::relativeTimeMicros();
//        if (videoInputs.ptsInRange(videoInputs.nextPts())) {
//          nextPts = videoInputs.nextPts();
//        }
//        spdlog::info("pts: {} nextPts: {}", vivictpp::time::formatTime(pts),
//                     vivictpp::time::formatTime(nextPts));
    }

    // Cleanup
    ImGui_ImplSDLRenderer_Shutdown();
    ImGui_ImplSDL2_Shutdown();

    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
