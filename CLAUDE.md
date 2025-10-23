# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Vivict++ is a video comparison tool for subjective visual quality assessment. It uses FFmpeg's libav* libraries to decode and display two video sources side-by-side with a split-view interface. The application is built with C++17, SDL2 for rendering, and Dear ImGui for the UI.

## Build System

This project uses **Meson** as the build system.

### Initial Setup

Before building, initialize git submodules (required since version 1.3):
```bash
git submodule update --init
```

Or clone with submodules:
```bash
git clone --recurse-submodules <repo-url>
```

### Building

**Ubuntu/Linux:**
```bash
# Create build directory with SDL2 subproject (recommended on Ubuntu)
meson -Duse_sdl2_subproject=true builddir

# Compile
meson compile -C builddir

# Executable will be at: builddir/vivictpp
```

**macOS:**
```bash
meson builddir
meson compile -C builddir
```

**Windows:**
Requires manual setup of dependencies in `winlibs/` folder. See README.md for details.

### Running Tests

```bash
# Run all tests
meson test -C builddir

# Run specific test
meson test -C builddir Settings
meson test -C builddir QualityMetrics
```

### Code Formatting

This project follows the **LLVM coding standard** with configuration in `.clang-format`:
```bash
# Format code (if you have clang-format installed)
clang-format -i <file>
```

Key style points:
- C++17 standard
- 80 character line limit
- 2-space indentation
- Pointer alignment: right (`int *ptr`)
- Brace style: attach (K&R style)

## Architecture Overview

### High-Level Components

**Entry Point (`src/main.cc`):**
- Parses command-line options via `OptParser`
- Loads settings and initializes logging
- Creates `VivictPPImGui` and enters main event loop

**UI Layer (`src/imgui/`):**
- `VivictPPImGui`: Main application class, orchestrates the UI
- `VideoWindow`: Renders video frames side-by-side with split view
- `Controls`: Handles playback controls
- `MainMenu`, `FileDialog`, `SettingsDialog`, `PlotWindow`: UI components
- `VideoMetadataDisplay`: Shows frame/stream metadata

**Playback Layer (`src/VideoPlayback.cc`):**
- `VideoPlayback`: Manages playback state (play/pause/seek)
- Coordinates timing and synchronization between video sources
- Handles speed adjustment and frame stepping

**Input Management (`src/VideoInputs.cc`):**
- `VideoInputs`: Manages left and right video sources
- Coordinates packet workers and decoder workers
- Handles seeking across multiple inputs
- Manages frame offset between left/right videos

### Worker Architecture

The video pipeline uses a **multi-threaded worker architecture**:

**Packet Workers (`src/workers/PacketWorker.cc`):**
- Thread that reads packets from input files via FFmpeg
- Feeds packets into queues for decoder workers
- Handles demuxing and stream selection

**Decoder Workers (`src/workers/DecoderWorker.cc`):**
- Thread that decodes video/audio packets into frames
- Applies FFmpeg filters if configured
- Maintains a `FrameBuffer` of decoded frames
- Handles seeking within the stream

**Frame Buffer (`src/workers/FrameBuffer.cc`):**
- Thread-safe circular buffer of decoded frames
- Provides frames to UI based on presentation timestamp (PTS)

**Data Flow:**
```
Input File → PacketWorker → PacketQueue → DecoderWorker → FrameBuffer → UI Rendering
```

### libav Abstraction Layer (`src/libav/`)

Wrappers around FFmpeg's libav* libraries:

- `FormatHandler`: Opens files, handles format context
- `Decoder`: Wraps codec context, decodes packets to frames
- `Filter`: Applies FFmpeg filters (scaling, format conversion, custom filters)
- `Frame` / `Packet`: RAII wrappers around AVFrame/AVPacket
- `HwAccelUtils`: Hardware acceleration utilities

### Video Quality Metrics (`src/qualitymetrics/`, `src/vmaf/`)

- Supports loading VMAF and other quality metrics from JSON/CSV files
- `QualityMetrics`: Generic interface for quality data
- `VmafLog`: Parser for VMAF JSON output
- Metrics can be plotted in `PlotWindow`

### Settings (`src/Settings.cc`)

- Persistent settings stored in TOML format
- Uses `tomlplusplus` library
- Location depends on platform (via `PlatformFolders`)

## Common Development Commands

### Build and Run
```bash
# Build
meson compile -C builddir

# Run with two videos
./builddir/vivictpp video1.mp4 video2.mp4

# Run with single video
./builddir/vivictpp video1.mp4

# Run with filters
./builddir/vivictpp --left-filter yadif source.mp4 transcoded.mp4
```

### Testing
```bash
# Run all tests
meson test -C builddir

# Run with verbose output
meson test -C builddir --verbose

# Run specific test file
./builddir/settingsTest
./builddir/qualitymetricsTest
```

### Development Workflow

When making changes:
1. Make your changes to source files
2. Run `meson compile -C builddir` to build
3. Run tests with `meson test -C builddir`
4. Test manually by running `./builddir/vivictpp`

## Key Implementation Details

### Time Representation

The project uses `vivictpp::time::Time` (microseconds as int64_t) for all time values:
- PTS (Presentation Timestamp) is in microseconds
- Conversion utilities in `src/time/TimeUtils.cc`
- Special value `vivictpp::time::NO_TIME` for undefined timestamps

### Thread Safety

- Workers use mutex-protected queues (`PacketQueue`, `FrameBuffer`)
- `QueuePointer` provides thread-safe read/write access
- Seeking requires coordination across all workers via callbacks

### Hardware Acceleration

- Configured via `DecoderOptions` and settings
- Supports various hwaccel types (auto, cuda, vaapi, etc.)
- Preferred decoders can be specified (e.g., `hevc_cuvid`, `libopenjpeg`)

### Video Resolution Handling

- When videos have different resolutions, the higher resolution is used
- Lower resolution video is upscaled
- Aspect ratio handling: prefers 16:9 if present, otherwise uses widest
- Custom filters can be used for precise control

## Git Workflow

- **Main branch:** `main`
- **Development branch:** `dev` (base all changes on this)
- All pull requests should target `dev`
- Sign commits with DCO: `git commit -s`
- Follow Git project commit message style (imperative mood, descriptive)
- Fix review comments by amending commits, not adding new commits
- Rebase instead of merge to resolve conflicts

## Dependencies

**Core:**
- FFmpeg libav* libraries (libavformat, libavcodec, libavfilter, libavutil, libswscale)
- SDL2 and SDL2_ttf
- Threads

**Third-party libraries (subprojects):**
- fmt, spdlog (logging)
- CLI11 (command-line parsing)
- Catch2 (testing)
- ImGui (UI framework)
- ImGuiFileDialog (file dialogs)
- implot (plotting)
- tomlplusplus (settings)
- PlatformFolders (cross-platform paths)

## Platform-Specific Notes

**Windows:**
- Requires manual dependency setup in `winlibs/`
- Uses UTF-8 command-line argument handling
- Uses `wWinMain` entry point

**Linux/Ubuntu:**
- Recommend using `-Duse_sdl2_subproject=true` to get recent SDL2 version
- Requires system FFmpeg libraries

**macOS:**
- Install via Homebrew for easiest setup
- May need to install `python@3.12` manually

## Testing Video Files

Test videos are in `testdata/` directory, generated with FFmpeg.
