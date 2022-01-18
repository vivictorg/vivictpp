// SPDX-FileCopyrightText: 2022 Sveriges Television AB
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include <ostream>
#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch2/catch.hpp"

extern "C" {
#include <SDL.h>
}


#include <vector>
#include <chrono>
#include <thread>
#include <iostream>
#include "SourceConfig.hh"
#include "Controller.hh"
#include "TimeUtils.hh"
#include <stdlib.h>

/*
  
  Mock eventLoop or mock VivictOOUI


*/

static int SDLCALL my_event_filter(void *userdata, SDL_Event * event)
{
    switch (event->type) {
    case SDL_QUIT:
    case SDL_MOUSEMOTION:
    case SDL_MOUSEWHEEL:
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
    case SDL_KEYDOWN:
        return 0;
    case SDL_WINDOWEVENT:
        return 1;
    }
    return 1;
}

class MockDisplay : vivictpp::ui::Display {
public:
    MockDisplay();
    virtual ~MockDisplay() = default;

    virtual void displayFrame(const std::array<vivictpp::libav::Frame, 2> &frames,
                              const vivictpp::ui::DisplayState &displayState) {
    }
    virtual int getWidth() {
        return 1920;
    }
    virtual int getHeight() {
        return 1080;
    }
    virtual void setFullscreen(bool fullscreen) {}
//  void setCursorHand();
//  void setCursorDefault();
    virtual void setLeftMetadata(const VideoMetadata &metadata) {

    }
    virtual void setRightMetadata(const VideoMetadata &metadata) {

    }
};


void mockKeyEvent(SDL_Keycode keycode) {
    SDL_Event event;
    event.type = SDL_KEYDOWN;
    event.key.keysym.sym = keycode;  //SDLK_SPACE
    SDL_PeepEvents(&event, 1, SDL_ADDEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);
}

void sleepSeconds(int n) {
    std::this_thread::sleep_for(std::chrono::seconds(n));
}

void sleepMillis(int n) {
    std::this_thread::sleep_for(std::chrono::milliseconds(n));
}

VivictPPConfig testConfig() {
    std::vector<SourceConfig> sourceConfigs;
    sourceConfigs.push_back(SourceConfig("/home/gugr01/local-work/test-video/ToS-4k-1920.mov"));
    sourceConfigs.push_back(SourceConfig("/home/gugr01/local-work/test-video/ToS-4k-1920.mov"));
    VivictPPConfig vivictPPConfig(sourceConfigs, true);
    return vivictPPConfig;
}

class EventFilterInitializer {
public:
    EventFilterInitializer() {
        SDL_SetEventFilter(my_event_filter, nullptr);
    }

    ~EventFilterInitializer() {
        SDL_SetEventFilter(nullptr, nullptr);
    }

private:
    vivictpp::sdl::SDLInitializer sdlInitializer;

};

class TestContext{
public:
    TestContext():
        eventFilterInitializer(),
        sdlEventLoop(std::make_shared<vivictpp::sdl::SDLEventLoop>(testConfig().sourceConfigs)),
        controller(sdlEventLoop, sdlEventLoop, testConfig()),
        thread(&vivictpp::Controller::run, &controller) {

    };

    ~TestContext() {
        controller.onQuit();
        thread.join();
    };

    EventFilterInitializer eventFilterInitializer;
    std::shared_ptr<vivictpp::sdl::SDLEventLoop> sdlEventLoop;
    vivictpp::Controller controller;
    std::thread thread;
};



TEST_CASE( "Seeking" ) {
    vivictpp::logging::initializeLogging();
    std::cout << "Creating test context" << std::flush << std::endl;
    TestContext testContext;
    sleepMillis(100);

    SECTION("Seek forward") {
        mockKeyEvent(SDLK_SLASH);
        mockKeyEvent(SDLK_SLASH);
        sleepMillis(500);
        double t = testContext.controller.getPlayerState().pts;

        REQUIRE( t == 10);
    }

    SECTION("Seek backward") {
        mockKeyEvent(SDLK_SLASH);
        mockKeyEvent(SDLK_SLASH);
        sleepMillis(200);
        mockKeyEvent(SDLK_m);
        sleepMillis(200);
        double t = testContext.controller.getPlayerState().pts;

        REQUIRE( t == 5);
    }

    SECTION("Seek backward beoynd start") {
        mockKeyEvent(SDLK_PERIOD);
        mockKeyEvent(SDLK_PERIOD);
        sleepMillis(200);
        mockKeyEvent(SDLK_m);
        sleepMillis(200);
        double t = testContext.controller.getPlayerState().pts;

        REQUIRE(t == 0);

    }
}

TEST_CASE( "Test playback" ) {
    vivictpp::logging::initializeLogging();
    TestContext testContext;
    sleepMillis(100);

    SECTION("Starts at pts 0") {
        double t = testContext.controller.getPlayerState().pts;

        REQUIRE(t == 0);
    }

    SECTION("Playback speed") {
        mockKeyEvent(SDLK_SPACE);
        uint64_t a0 = testContext.controller.getPlayerState().lastFrameAdvance;
        while (a0 == std::numeric_limits<uint64_t>::min()) {
            sleepMillis(1);
            a0 = testContext.controller.getPlayerState().lastFrameAdvance;
        }
        double t0 = testContext.controller.getPlayerState().pts;
        uint64_t b0 = vivictpp::util::relativeTimeMicros();
        sleepSeconds(10);

        double t1 = testContext.controller.getPlayerState().pts;
        uint64_t a1 = testContext.controller.getPlayerState().lastFrameAdvance;
        uint64_t b1 = vivictpp::util::relativeTimeMicros();
        uint64_t playerElapsedTimeMillis = static_cast<uint64_t>(1000 * (t1 - t0)) + (b1 - a1) / 1000;

        int drift = std::abs(static_cast<int>(playerElapsedTimeMillis) - 10000);

        REQUIRE(drift < 10);

    }

}
