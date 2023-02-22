All notable changes to this project will be documented in this file.
We follow the [Semantic Versioning 2.0.0](http://semver.org/) format.

## Current

## 0.2.5 - 2023-02-22

### Changed
- Snap updated to use ffmpeg5

## 0.2.4 - 2023-01-05

### Added
- Support for setting preferred decoders
- Support for hardware accelerated decoding

## 0.2.3 - 2022-10-16

## Fixed
- Fixed incorrect logging statement that caused problem when building on arch
- Fixed dependency ordering in meson.build that caused problem when building on arch

## 0.2.2 - 2022-10-16

### Changed
- fmt library subproject is now build as static library
- Dependencies on sdl2 and sdl2_ttf are now resolved from system as default
- Failed seek operations now handled gracefully
- Playback of hls live manifests now works

## 0.2.1 - 2022-09-23

### Added
 - Scaling of fonts based on display dpi
 - Scaling of content to fit window
 - Control of playback speed

## 0.2.0 - 2022-08-20

### Added
- Implement proper seekbar

### Changed
- Updated to C++17

## 0.1.14 2022-07-07

### Added
- Support for filters that change video resolution

## 0.1.13 2022-06-10

### Added
- Audio is now disabled by default
- Logav loglevel can now be specified through env variable
- Thread option for decoder is now set to auto
- Playing streams without pts data is now supported

## 0.1.12 2022-03-18

### Added
- Support for specifying input format through --left-format and --right-format commandline options
- Support for estimating pts from frame rate if not set in frames
- Support for compiling against ffmpeg5 libraries

## 0.1.11 2022-03-04

### Added
- Shift and Alt-shift can now be used togther with 'm' and '/' to seek 1 minute or 10 minute
  forward or backward.
- Support for left stream offset: shift-, and shift-. can now be used to offset the left stream by an integer number
of frames.

### Fixed
- int64_t datatype now used for timestamps to avoid rounding errors

## 0.1.10 2022-02-17

### Added
- Added removable-media slot

## 0.1.9 2022-02-05

### Fixed
- Seeking backwards now stops on stream start
- Playback stops on stream end
- Seeking forward stops on stream end
- Resolved problems with playback after multiple seeks

## x.y.z - YYYY-MM-DD

### Added
- Lorem ipsum dolor sit amet

### Deprecated
- Nothing.

### Removed
- Nothing.

### Fixed
- Nothing.
