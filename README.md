[![REUSE status](https://api.reuse.software/badge/github.com/vivictorg/vivictpp)](https://api.reuse.software/info/github.com/vivictorg/vivictpp)

    ____   ____._______   ____._______________________                        
    \   \ /   /|   \   \ /   /|   \_   ___ \__    ___/    .__         .__     
     \   Y   / |   |\   Y   / |   /    \  \/ |    |     __|  |___   __|  |___ 
      \     /  |   | \     /  |   \     \____|    |    /__    __/  /__    __/ 
       \___/   |___|  \___/   |___|\______  /|____|       |__|        |__|    
                                          \/
         VIVICT VIDEO COMPARISON TOOL ++

An easy to use tool for subjective comparison of the visual quality of different encodings of the same video source.

![](img/screenshot.png?raw=true "vivict++ screenshot")
(_Screenshot contains images from [Tears Of Steel](https://mango.blender.org), (CC) Blender Foundation, licensed under [Creative Commons Attribution License](https://creativecommons.org/licenses/by/3.0/)_ )




Vivict++ is a sibling to [vivict](http://github.com/svt/vivict). Vivict++ is not limited by browser support for different video format and codecs, and therefore, it is able to support a much wider range of formats and codecs, due to the power of the FFmpeg libav\*libraries. Basically, Vivict++ supports any format/codec supported by ffmpeg.

__*Note that this software is in an alpha state. Bugs can be expected,
the code is not as clean as one would wish etc. See [known issues](#known-issues)*__

## Installation

### Snap

```
sudo snap install --edge vivictpp
```
Note that due to snap security features, vivictpp will only be able to access files in your home folder
when installed from snap. Connecting the vivictpp snap to the systems removable-media slot will allow
also accessing files in location under /mnt and /media. The below command can be used for this.
```
sudo snap connect vivictpp:removable-media :removable-media
```

### Homebrew
```
brew tap grusell/vivictpp
brew install vivictpp
```

## Building

### Ubuntu

1. Install dependencies

```console

$ apt-get --fix-missing install -y cmake python3-pip gcc python3-setuptools \
  python3-wheel libfreetype6-dev libavformat-dev libavcodec-dev \
  libavfilter-dev libswscale-dev pkg-config

```

```console

$ pip3 install ninja meson

```

2. Create meson builddir

```console

$ meson -Duse_sdl2_subproject=true builddir

```

(Setting the `use_sdl2_subproject` option will cause meson to build sdl2 and sdl2_ttf
as subprojects. This is recomended on ubuntu since the sdl2 version provided by ubuntu
is not the most recent one)

3. Compile

```console

$ meson compile -C builddir

```

4. The `vivictpp` executable should now be available in `builddir`

### Other platforms

Vivict++ is intended to be cross platform, so building on Windows or macOS should be possible.
Testing and help would be appreciated.

## Usage
Run with the -h flag to see the commandline options

    > vivictpp -h
    Vivict++ - Vivict Video Comparison Tool ++
    Usage: ./build/vivictpp [OPTIONS] leftVideo [rightVideo]
    
    Positionals:
      leftVideo TEXT REQUIRED     Path or url to first (left) video
      rightVideo TEXT             Path or url to second (right) video
    
    Options:
      -h,--help                   Print this help message and exit
      --left-filter TEXT          Video filters for left video
      --right-filter TEXT         Video filters for left video
      --enable-audio              Enable audio
      --left-vmaf TEXT            Path to csv-file containing vmaf data for left video
      --right-vmaf TEXT           Path to csv-file containing vmaf data for right video
      --left-format TEXT          Format options for left video input
      --right-format TEXT         Format options for right video input
      --disable-font-autoscaling  Disables autoscaling of fonts based on display dpi
      --custom-font-scaling FLOAT Custom scaling factor for fonts
      --hwaccel TEXT              Select device type to use for hardware accelerated decoding. Valid values are:
                                    auto    Use any available device type (default)
                                    none    Disable hardware accelerated decoding
                                    TYPE    Name of devicetype, see https://trac.ffmpeg.org/wiki/HWAccelIntro
      --preferred-decoders TEXT   Comma separated list of decoders that should be preferred over default decoder when applicable
      --imgui,--disable-imgui{false}
                                  Enable experimental ImGui UI
    
    
    KEYBOARD SHORTCUTS
    
    SPACE  Play/Pause video
    ,      Step forward 1 frame
    .      Step backward 1 frame
    / or - Seek forward 5 seconds
    m      Seek backward 5 seconds
    ?      Seek Forward 60s
    M      Seek backward 60s
    Alt-?  Seek Forward 10min
    Alt-M  Seek Backward 10min
    <      Decrease left frame offset
    >      Increase left frame offset
    [      Decrease playback speed
    ]      Increase playback speed
    
    f      Toggle full screen
    u      Zoom in
    i      Zoom out
    0      Reset pan and zoom to default
    s      Toggle scale content to fit window
    t      Toggle visibility of time
    d      Toggle visibility of Stream and Frame metadata
    p      Toggle visibility of vmaf plot (if vmaf data present)
    
    q      Quit application
    
    See also  https://github.com/svt/vivictpp#readme
    
    Vivict++ 0.2.5 


To visually compare two videos in files (or urls) VIDEO1 and VIDEO2 respectively:

    > vivictpp VIDEO1 VIDEO2

Vivict++ can also be run with only one video

    > vivictpp VIDEO1

### Controlling playback and view using the mouse

* The split line is controlled by simply moving the mouse left and right
* Panning is done by left clicking and dragging
* Left clicking anywhere starts/stops the playback

### Using video filters
The `--left-filter` and `--right-filter` options can be used to specify ffmpeg filters that are applied to the input(s) before being displayed. The filters should be specified using ffmpeg filter syntax, see [FFmpeg Filters Documentation](http://ffmpeg.org/ffmpeg-filters.html)

For example, to compare an interlaced source with a transcoded and deinterlaced variant, the below command could be used

    > vivictpp --left-filter yadif SOURCEVIDEO TRANSCODEDVIDEO


### Hardware accelerated decoding (experimental)
There are two kind of hardware accelerated decoders in ffmpeg/libav, internal hwaccel decoders and external wrapper
decoders. Vivict++ has support for both, allthough this should be considered an experimental feature.

To use libav internal hwaccel decoders, the `--hwaccel` option can be used. By default it is set to `none` which mean
internal hardware accelerated decoders will be disabled. If `auto` is passed to the `--hwaccel` option any available api
for hardware accelerated decoding will be used. It is also possible to specify a specific hardware acceleration api to use
, such as `vaapi`, `videotoolbox` etc.

To use external wrapper decoders, use the `--preferred-decoders` option (see below) to specify the decoders to use.

For more info on harware accelerating decoding, see https://trac.ffmpeg.org/wiki/HWAccelIntro .

Note that even with hardware accelerated decoding, depending of the format of the source video, vivict++ might still
use quite a bit of cpu to do pixel format conversion. With `--hwaccel cuda` or `--hwaccel vaapi` pixel format conversion may be done on
the gpu.

### Specify preferred decoder
In some cases it might be preferable to use a different decoder than the libav default. For instance, `libopenjpeg` may give
better performance than libav's native jpeg2000 decoder, and one may want to use `hevc_cuvid` instead of the x265 decoder.
A comma separated list of preferred codecs can be specified on the commandline with the `--preferred-decoders` option.
The codec name should be as listed by `ffmpeg -codecs` . The first, if any, of the listed preferred codecs that is aplicable
for the input will be used for decoding. Non-applicable codecs will simply be ignored.

For instance, to use cuvid decoders for h264 and h265, and libopenjpeg for jpeg 2000 input,  one would call vivictpp like below.

```
vivictpp --preferred-decoders h264_cuvid,hevc_cuvid,libopenjpeg video.mp4
```

### Displaying video quality (VMAF) data
_*Note that this functionality is currently not available in the new imgui UI. To use this functionality vivictpp must be started with the --disable-imgui flag*_

Vivict++ can display VMAF data if such is provided by using the `--left-vmaf` and/or `--right-vmaf`
options. The command line options take a path to a csv-file containing the data as argument. The file is expected to be a csv-file created by the [FFmpeg libvmaf filter](http://ffmpeg.org/ffmpeg-filters.html#libvmaf) with `log_fmt=csv`.

### Specifying input format
In case your input file is in a format this not easily identified, ie raw video, you can use the `--left-format` and `--right-format` to tell vivictpp how to interpret the input file. These options should be followed by a colon-separated list of key=value pairs. If `format` is specified as key, the corresponding value will be passed to [av_find_input_format](https://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html#ga40034b6d64d372e1c989e16dde4b459a) to find the correct input format. Any other key value pairs will be passed to [avformat_open_input](https://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html#gac05d61a2b492ae3985c658f34622c19d) through the `options` parameter.

Example for playing a file containing raw video data:

    vivictpp --left-format format=rawvideo:pixel_format=yuv422p10:video_size=1280x720:framerate=50 my-file.yuv

### Controlling playback speed
Playback speed can be controlled with `[` and `]`.

### Logging

Logging for debugging purposes can be enabled by setting the environment variable `SPDLOG_LEVEL` to `DEBUG` or even `TRACE`.

Some classes have a named logger, setting for a specific logger can be done like below

    SPDLOG_LEVEL=info,DecoderWorker=debug vpp file1 file2

The log level for the libav libraries can be specified by setting the environment variable `AVLOG_LEVEL`. Valid values are
`quiet, panic, fatal, error, warning, info, verbose, debug, trace`.

### Audio
Audio is currently not supported in the new imgui UI. If vivictpp is started with the `--disable-imgui` flag, audio is supported.

## Known issues

* Audio sync is less than perfect

## Code standard

This project more or less follows the LLVM standard.

## Contributing

Contributions are welcome! See [Contributing](docs/CONTRIBUTING.md), as well as the [Code of Conduct](docs/CODE_OF_CONDUCT.md).

## Test videos

The testdata folder contains some testdata generated with FFmpeg.


## Third Party Dependencies
Vivict++ uses the following thirdparty dependencies.

* [FFmpeg libav\*libraries like libavcodec, libavfilter and more](https://ffmpeg.org/) ([GNU General Public License (GPL) version 2 or later (OR LGPL)](https://www.ffmpeg.org/legal.html))
* [SDL2](https://www.libsdl.org/) ([zlib License](https://www.libsdl.org/license.php))
* [SDL2_ttf](https://www.libsdl.org/projects/SDL_ttf/) ([zlib License](https://github.com/libsdl-org/SDL_ttf/blob/main/COPYING.txt))
* [ImGui](https://github.com/ocornut/imgui) ([MIT License](https://github.com/ocornut/imgui/blob/master/LICENSE.txt))
* [ImGuiFileDialog](https://github.com/aiekick/ImGuiFileDialog) ([MIT License](https://github.com/aiekick/ImGuiFileDialog/blob/master/LICENSE))
* [spdlog](https://github.com/gabime/spdlog) ([MIT License](https://github.com/gabime/spdlog/blob/v1.x/LICENSE))
* [cli11](https://github.com/CLIUtils/CLI11) ([The 3-Clause BSD License](https://raw.githubusercontent.com/CLIUtils/CLI11/master/LICENSE))
* [Catch2](https://github.com/catchorg/Catch2) ([Boost Software License 1.0](https://raw.githubusercontent.com/catchorg/Catch2/devel/LICENSE.txt))
* [FreeMono from GNU freefont](http://savannah.gnu.org/projects/freefont/) ([GPL License](https://www.gnu.org/software/freefont/license.html))
* [vivict-icons](https://github.com/vivictorg/vivict-icons) ([MIT License](https://github.com/vivictorg/vivict-icons/blob/main/LICENSE))

## License

Vivict++ is licensed under the GNU General Public License version 2, or later. 
Derivative work may be relicensed to a later version of GPL. See [LICENSE](LICENSE).
