// Dear ImGui: standalone example application for SDL2 + SDL_Renderer
// (SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

// Important to understand: SDL_Renderer is an _optional_ component of SDL. We do not recommend you use SDL_Renderer
// because it provides a rather limited API to the end-user. We provide this backend for the sake of completeness.
// For a multi-platform app consider using e.g. SDL+DirectX on Windows and SDL+OpenGL on Linux/OSX.


#include "SDL_render.h"
#include "SDL_video.h"
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
void ScrollWhenDraggingOnVoid(const ImVec2& delta, ImGuiMouseButton mouse_button)
{
    ImGuiContext& g = *ImGui::GetCurrentContext();
    ImGuiWindow* window = g.CurrentWindow;
    ImGuiID id = window->GetID("##scrolldraggingoverlay");
    ImGui::KeepAliveID(id);
    bool hovered = false;
    bool held = false;
    ImGuiButtonFlags button_flags = (mouse_button == 0) ? ImGuiButtonFlags_MouseButtonLeft : (mouse_button == 1) ? ImGuiButtonFlags_MouseButtonRight : ImGuiButtonFlags_MouseButtonMiddle;
    if (g.HoveredId == 0) // If nothing hovered so far in the frame (not same as IsAnyItemHovered()!)
        ImGui::ButtonBehavior(window->Rect(), id, &hovered, &held, button_flags);
    if (held && delta.x != 0.0f)
        ImGui::SetScrollX(window, window->Scroll.x - delta.x);
    if (held && delta.y != 0.0f)
        ImGui::SetScrollY(window, window->Scroll.y - delta.y);
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

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    // Our state
    ImVec4 clear_color = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);

    // Main loop
    bool done = false;
    bool fullscreen = false;
    bool playing = true;


    VideoInputs videoInputs(vivictPPConfig);

    vivictpp::ui::DisplayState displayState;
    displayState.splitScreenDisabled = true;
    displayState.leftVideoMetadata = videoInputs.metadata()[0][0];
    
    vivictpp::sdl::SDLTexture texture(renderer, 1920, 800, SDL_PIXELFORMAT_YV12);
    
    
    vivictpp::time::Time pts{0};
    vivictpp::time::Time nextPts{0};

    int64_t t0 = 0;
    int64_t pts0 = videoInputs.startTime();
    int64_t tLastPresent = 0;

    if (playing) {
      t0 = vivictpp::time::relativeTimeMicros();
    }
    
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
//          spdlog::info("windowSize: {}x{}", windowSize.x, windowSize.y);
          const float PAD = 10.0f;
          const ImGuiViewport* viewport = ImGui::GetMainViewport();
          ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
          ImVec2 work_size = viewport->WorkSize;

          ImGui::SetNextWindowPos({0,0});
          ImGui::SetNextWindowSize(work_size);
          int scaleFactor = 2;
          ImVec2 videoSize(1920, 800);
          ImVec2 scaledVideoSize = {videoSize.x * scaleFactor, videoSize.y * scaleFactor};
          ImGui::SetNextWindowContentSize(scaledVideoSize);
          bool myBool2;
          if (ImGui::Begin("Test window", &myBool2,  ImGuiWindowFlags_AlwaysAutoResize |
                           ImGuiWindowFlags_NoDecoration |
                           ImGuiWindowFlags_NoSavedSettings |
                           ImGuiWindowFlags_NoFocusOnAppearing |
                           ImGuiWindowFlags_NoNav |
                           ImGuiWindowFlags_NoBringToFrontOnFocus |
                           ImGuiWindowFlags_NoTitleBar |
                           ImGuiWindowFlags_NoScrollbar
                )) {
            ImGui::Button("TESTING");
            nextPts = videoInputs.nextPts();
            if (playing && videoInputs.ptsInRange(nextPts)) {
              int64_t t = vivictpp::time::relativeTimeMicros();
//              spdlog::debug("pts: {} nextPts: {}", vivictpp::time::formatTime(pts), vivictpp::time::formatTime(nextPts));
//              spdlog::debug("t - t0 = {}, nextPts - pts = {}", t - t0, nextPts - pts0);
              int64_t tNextPresent = tLastPresent + (int64_t) (1e6 * io.DeltaTime);
              if ((tNextPresent - t0) >= (nextPts - pts0)) {
                pts = nextPts;
                videoInputs.stepForward(nextPts);
//                spdlog::info("Step forward");
              }
            }
            texture.update(videoInputs.firstFrames()[0]);
            int w,h;
            SDL_GetRendererOutputSize(renderer, &w, &h);
            //ImVec2 drawSize(windowWidth, windowHeight);
            ImVec2 viewSize(w, h);
            ImVec2 drawSize(w, h);
//            spdlog::info("drawSize: {}x{}", drawSize.x, drawSize.y);

            drawSize.x *= scaleFactor;
            drawSize.y *= scaleFactor;
            
            float xScale = drawSize.x / videoSize.x;
            float yScale = drawSize.y / videoSize.y;
            if (xScale < 1 || yScale < 1) {
              if (xScale < yScale) {
                drawSize.x = windowWidth;
                drawSize.y = videoSize.y * windowWidth / videoSize.x;
              } else {
                drawSize.x = videoSize.x * windowHeight / videoSize.y;
                drawSize.y = windowHeight;
              }
            }
            float scrollX = ImGui::GetScrollX();
            float scrollY = ImGui::GetScrollY();
//            spdlog::info("scroll: {},{}", scrollX, scrollY);
//            ImVec2 cursorPos = ImGui::GetCursorScreenPos();
//            spdlog::info("cursorPos: {},{}", cursorPos.x, cursorPos.y);
//            ImVec2 drawPos(cursorPos.x + 0.5 * (windowWidth - drawSize.x), cursorPos.y + 0.5 * (windowHeight - drawSize.y));
            ImVec2 drawPos = {0, 0}; //cursorPos;
            ImVec2 uvMin(scrollX / scaledVideoSize.x, scrollY/ scaledVideoSize.y);
            ImVec2 uvMax( std::min((float) 1.0, (scrollX + viewSize.x) / scaledVideoSize.x), std::min((float) 1, (scrollY + viewSize.y) / scaledVideoSize.y));
//            spdlog::info("uvMin: {},{}", uvMin.x, uvMin.y);
//            spdlog::info("uvMax: {},{}", uvMax.x, uvMax.y);
            ImVec2 p2(drawPos.x + viewSize.x, drawPos.y + viewSize.y);
//            ImGui::Image((void*)(intptr_t)texture.get(), drawPos, p2);
            ImGui::GetWindowDrawList()->AddImage((void*)(intptr_t)texture.get(),
                                                 drawPos, p2, uvMin, uvMax);
            //ImGui::GetBackgroundDrawList()->AddImage((void*)(intptr_t)texture.get(),
            //                                         drawPos, p2);
            ImVec2 mouse_delta = ImGui::GetIO().MouseDelta;
            ScrollWhenDraggingOnVoid(mouse_delta, 0);
          }
          ImGui::End();

              
          ImVec2 window_pos, window_pos_pivot;
          window_pos.x = work_pos.x + work_size.x / 2;
          window_pos.y = work_size.y - PAD;
          window_pos_pivot.x = 0.5f;
          window_pos_pivot.y = 1.0f;
          ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
          ImGui::SetNextWindowSize({0.0, 0.0});
          window_flags |= ImGuiWindowFlags_NoMove;
          ImGui::SetNextWindowBgAlpha(0.10f); // Transparent background
          bool myBool;

          if (ImGui::Begin("Video controls", &myBool, window_flags)) {
            ImVec2 windowSize = ImGui::GetWindowSize();
//            spdlog::info("windowSize: {}x{}", windowSize.x, windowSize.y);
                
            if (ImGui::Button(playing ? "Pause" : "Play")) {
              pts0 = pts;
              t0 = vivictpp::time::relativeTimeMicros();
              playing = !playing;
            }
            ImGui::SameLine();
            if (ImGui::Button("Step")) {
              if (videoInputs.ptsInRange(videoInputs.nextPts())) {
                pts = videoInputs.nextPts();
                //                  spdlog::info("Stepping to {}", pts);
                videoInputs.stepForward(pts);
              }
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
