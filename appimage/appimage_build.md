# Building vivictpp AppImage

This guide provides multiple methods to create an AppImage for [vivictpp](https://github.com/vivictorg/vivictpp), a video comparison tool.

## Prerequisites

### System Requirements
- Linux operating system (x86_64)
- Internet connection for downloading dependencies

### Required Tools
The build process requires several tools and libraries. The exact installation commands depend on your distribution:

#### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    git \
    meson \
    ninja-build \
    pkg-config \
    python3-pip \
    libfreetype6-dev \
    libavformat-dev \
    libavcodec-dev \
    libavfilter-dev \
    libswscale-dev \
    libsdl2-dev \
    libsdl2-ttf-dev
```

#### Fedora/CentOS/RHEL
```bash
sudo dnf install -y \
    gcc gcc-c++ \
    git \
    meson \
    ninja-build \
    pkgconfig \
    python3-pip \
    freetype-devel \
    ffmpeg-devel \
    SDL2-devel \
    SDL2_ttf-devel
```

#### Arch Linux
```bash
sudo pacman -S \
    base-devel \
    git \
    meson \
    ninja \
    pkgconf \
    python-pip \
    freetype2 \
    ffmpeg \
    sdl2 \
    sdl2_ttf
```

## Method 1: Direct Build Script

Use the provided build script for a straightforward build process:

1. **Download the build script** (use the first artifact above)
2. **Make it executable:**
   ```bash
   chmod +x build_vivictpp_appimage.sh
   ```
3. **Run the build:**
   ```bash
   ./build_vivictpp_appimage.sh
   ```

The script will:
- Check for required dependencies
- Clone the vivictpp repository
- Build using meson/ninja
- Create the AppDir structure
- Bundle dependencies
- Generate the final AppImage

## Method 2: Docker Build (Recommended)

For a more reproducible build environment, use Docker:

1. **Create the build directory:**
   ```bash
   mkdir vivictpp-appimage-build
   cd vivictpp-appimage-build
   ```

2. **Save the Dockerfile** (use the second artifact above)

3. **Copy the build script** into the same directory as `build_appimage.sh`

4. **Build the Docker image:**
   ```bash
   docker build -t vivictpp-builder .
   ```

5. **Run the build:**
   ```bash
   docker run --rm -v $(pwd)/output:/home/builder/output vivictpp-builder
   ```

## Method 3: Manual Build

For those who prefer manual control or need to customize the build:

### Step 1: Clone the Repository
```bash
git clone https://github.com/vivictorg/vivictpp.git
cd vivictpp
```

### Step 2: Configure the Build
```bash
meson setup build \
    --prefix=/usr \
    --bindir=bin \
    --libdir=lib \
    -Duse_sdl2_subproject=true \
    --buildtype=release
```

### Step 3: Compile
```bash
meson compile -C build
```

### Step 4: Create AppDir Structure
```bash
mkdir -p vivictpp.AppDir/usr/{bin,lib,share/{applications,icons/hicolor/scalable/apps}}
```

### Step 5: Install Binary and Dependencies
```bash
# Copy the binary
cp build/vivictpp vivictpp.AppDir/usr/bin/

# Copy dependencies (you'll need to identify and copy required .so files)
# This is the most complex part and varies by system
```

### Step 6: Create Desktop File and Icon
Create `vivictpp.AppDir/vivictpp.desktop`:
```ini
[Desktop Entry]
Type=Application
Name=Vivict++
Comment=Video comparison tool
Exec=vivictpp
Icon=vivictpp
Categories=AudioVideo;Video;
```

### Step 7: Create AppRun Script
```bash
#!/bin/bash
SELF=$(readlink -f "$0")
HERE=${SELF%/*}
export LD_LIBRARY_PATH="${HERE}/usr/lib:${LD_LIBRARY_PATH}"
exec "${HERE}/usr/bin/vivictpp" "$@"
```

### Step 8: Generate AppImage
```bash
# Download appimagetool
wget "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage"
chmod +x appimagetool-x86_64.AppImage

# Create AppImage
./appimagetool-x86_64.AppImage vivictpp.AppDir
```

## Troubleshooting

### Common Issues

1. **Missing Dependencies**
   - Ensure all development libraries are installed
   - Check that pkg-config can find the libraries: `pkg-config --list-all | grep -E "(avformat|avcodec|sdl2)"`

2. **Meson Configuration Fails**
   - Try using the subproject option: `-Duse_sdl2_subproject=true`
   - Check meson log: `cat build/meson-logs/meson-log.txt`

3. **Runtime Issues**
   - Test the binary before packaging: `./build/vivictpp --help`
   - Check missing libraries: `ldd build/vivictpp`

4. **AppImage Won't Run**
   - Verify permissions: `chmod +x vivictpp-*.AppImage`
   - Test with verbose output: `./vivictpp-*.AppImage --help`

### Build Customization

#### Optimized Build
```bash
meson setup build \
    -Duse_sdl2_subproject=true \
    --buildtype=release \
    --optimization=3 \
    --strip
```

#### Debug Build
```bash
meson setup build \
    -Duse_sdl2_subproject=true \
    --buildtype=debug
```

## Testing the AppImage

Once built, test your AppImage:

```bash
# Check basic functionality
./vivictpp-*.AppImage --help

# Test with sample videos (if available)
./vivictpp-*.AppImage video1.mp4 video2.mp4
```

## Distribution

The resulting AppImage is a single, portable file that can be:
- Distributed to users without requiring installation
- Run on most Linux distributions
- Integrated with desktop environments automatically

## Additional Resources

- [vivictpp GitHub Repository](https://github.com/vivictorg/vivictpp)
- [AppImage Documentation](https://docs.appimage.org/)
- [Meson Build System](https://mesonbuild.com/)
- [FFmpeg Development Libraries](https://ffmpeg.org/download.html)

## Contributing

If you improve this build process or encounter issues, consider contributing back to the vivictpp project or sharing your improvements with the community.
