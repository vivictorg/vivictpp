[![REUSE status](https://api.reuse.software/badge/github.com/svt/vivictpp)](https://api.reuse.software/info/github.com/svt/vivictpp)

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

Coming soon...

## Building

### Ubuntu

1. Install dependencies

```console

$ apt-get --fix-missing install -y cmake python3-pip gcc python3-setuptools \
  python3-wheel libsdl2-dev libsdl2-ttf-dev libfreetype6-dev libavformat-dev libavcodec-dev \
  libswscale-dev pkg-config

```

```console

$ pip3 install ninja meson

```

2. Create meson builddir

```console

$ meson builddir

```

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
    Usage: vpp [OPTIONS] leftVideo [rightVideo]
    
    Positionals:
      leftVideo TEXT REQUIRED     Path or url to first (left) video
      rightVideo TEXT             Path or url to first (right) video
    
    Options:
      -h,--help                   Print this help message and exit
      --left-filter TEXT          Video filters for left video
      --right-filter TEXT         Video filters for left video
      --disable-audio             Disable audio
      --left-vmaf TEXT            Path to csv-file containing vmaf data for left video
      --right-vmaf TEXT           Path to csv-file containing vmaf data for right video

    KEYBOARD SHORTCUTS
    
    SPACE  Play/Pause video
    ,      Step forward 1 frame
    .      Step backward 1 frame
    / or - Step forward 5 seconds
    m      Step backward 5 seconds
    
    f      Toggle full screen
    u      Zoom in
    i      Zoom out
    0      Reset pan and zoom to default
    t      Toggle visibility of time
    d      Toggle visibility of Stream and Frame metadata
    p      Toggle visibility of vmaf plot (if vmaf data present)
    
    q      Quit application
    
    See also  https://github.com/svt/vivictpp#readme


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


### Displaying video quality (VMAF) data

Vivict++ can display VMAF data if such is provided by using the `--left-vmaf` and/or `--right-vmaf`
options. The command line options take a path to a csv-file containing the data as argument. The file is expected to be a csv-file created by the [FFmpeg libvmaf filter](http://ffmpeg.org/ffmpeg-filters.html#libvmaf) with `log_fmt=csv`.

### Logging

Logging for debugging purposes can be enabled by setting the environment variable `SPDLOG_LEVEL` to `DEBUG` or even `TRACE`.

Some classes have a named logger, setting for a specific logger can be done like below

    SPDLOG_LEVEL=info,DecoderWorker=debug vpp file1 file2

## Known issues

* Audio sync is less than perfect

## Code standard

This project more or less follows the LLVM standard.

## Contributing

Contributions are welcome! See [Contributing](docs/CONTRIBUTING.md), as well as the [Code of Conduct](docs/CODE_OF_CONDUCT.md).

## Test videos

The testdata folder contains some testdata generated with FFmpeg.


## Third Party Dependencies
Vivict++ uses the following thirdparty libraries.

* [FFmpeg libav\*libraries like libavcodec, libavfilter and more](https://ffmpeg.org/) ([GNU General Public License (GPL) version 2 or later (OR LGPL)](https://www.ffmpeg.org/legal.html))
* [SDL2](https://www.libsdl.org/) ([zlib License](https://www.libsdl.org/license.php))
* [SDL2_ttf](https://www.libsdl.org/projects/SDL_ttf/) ([zlib License](https://github.com/libsdl-org/SDL_ttf/blob/main/COPYING.txt))
* [spdlog](https://github.com/gabime/spdlog) ([MIT License](https://github.com/gabime/spdlog/blob/v1.x/LICENSE))
* [cli11](https://github.com/CLIUtils/CLI11) ([The 3-Clause BSD License](https://raw.githubusercontent.com/CLIUtils/CLI11/master/LICENSE))
* [Catch2](https://github.com/catchorg/Catch2) ([Boost Software License 1.0](https://raw.githubusercontent.com/catchorg/Catch2/devel/LICENSE.txt))

## License

Vivict++ is licensed under the GNU General Public License version 2, or later. 
Derivative work may be relicensed to a later version of GPL. See [LICENSE](LICENSE).
