#!/bin/bash
set -e

# vivictpp AppImage Build Script
# This script builds vivictpp from source and packages it into an AppImage

# Configuration
APPNAME="vivictpp"
VERSION="latest"
ARCH="x86_64"
BUILD_DIR="build"
APPDIR="${APPNAME}.AppDir"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

log() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

error() {
    echo -e "${RED}[ERROR]${NC} $1"
    exit 1
}

# Check if running on Linux
if [[ "$OSTYPE" != "linux-gnu"* ]]; then
    error "This script must be run on Linux to create AppImages"
fi

# Function to check if a command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Check for required tools
log "Checking for required tools..."
MISSING_TOOLS=()

if ! command_exists git; then
    MISSING_TOOLS+=("git")
fi

if ! command_exists meson; then
    MISSING_TOOLS+=("meson")
fi

if ! command_exists ninja; then
    MISSING_TOOLS+=("ninja")
fi

if ! command_exists pkg-config; then
    MISSING_TOOLS+=("pkg-config")
fi

if ! command_exists gcc; then
    MISSING_TOOLS+=("gcc")
fi

if [[ ${#MISSING_TOOLS[@]} -ne 0 ]]; then
    error "Missing required tools: ${MISSING_TOOLS[*]}. Please install them first."
fi

# Check for required development libraries
log "Checking for required development libraries..."
MISSING_LIBS=()

# Check using pkg-config
for lib in libavformat libavcodec libavfilter libswscale; do
    if ! pkg-config --exists $lib; then
        MISSING_LIBS+=("$lib")
    fi
done

if [[ ${#MISSING_LIBS[@]} -ne 0 ]]; then
    warn "Missing development libraries: ${MISSING_LIBS[*]}"
    warn "On Ubuntu/Debian, install with:"
    warn "sudo apt-get install libfreetype6-dev libavformat-dev libavcodec-dev libavfilter-dev libswscale-dev"
fi

# Download appimagetool if not present
if ! command_exists appimagetool; then
    log "Downloading appimagetool..."
    wget -O appimagetool "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage"
    chmod +x appimagetool
    APPIMAGETOOL="./appimagetool"
else
    APPIMAGETOOL="appimagetool"
fi

# Clean up previous builds
log "Cleaning up previous builds..."
rm -rf "$BUILD_DIR" "$APPDIR" "${APPNAME}-${VERSION}-${ARCH}.AppImage"

# Clone or update the repository
if [ ! -d "vivictpp" ]; then
    log "Cloning vivictpp repository..."
    git clone https://github.com/vivictorg/vivictpp.git
else
    log "Updating vivictpp repository..."
    cd vivictpp
    git pull
    cd ..
fi

cd vivictpp

# Get version from git if possible
if command_exists git && git rev-parse --git-dir > /dev/null 2>&1; then
    VERSION=$(git describe --tags --always --dirty 2>/dev/null || echo "unknown")
fi

log "Building vivictpp version: $VERSION"

# Configure the build with meson
log "Configuring build with meson..."
meson setup "$BUILD_DIR" \
    --prefix=/usr \
    --bindir=bin \
    --libdir=lib \
    -Duse_sdl2_subproject=true \
    --buildtype=release

# Build the project
log "Building vivictpp..."
meson compile -C "$BUILD_DIR"

# Create AppDir structure
log "Creating AppDir structure..."
cd ..
mkdir -p "$APPDIR/usr/bin"
mkdir -p "$APPDIR/usr/lib"
mkdir -p "$APPDIR/usr/share/applications"
mkdir -p "$APPDIR/usr/share/icons/hicolor/scalable/apps"
mkdir -p "$APPDIR/usr/share/pixmaps"

# Install the binary
log "Installing vivictpp binary..."
cp "vivictpp/$BUILD_DIR/vivictpp" "$APPDIR/usr/bin/"

# Copy dependencies
log "Copying dependencies..."
copy_deps() {
    local binary="$1"
    local dest_dir="$2"
    
    # Get list of shared library dependencies
    local deps=$(ldd "$binary" 2>/dev/null | grep "=> /" | awk '{print $3}' | grep -v "^/lib" | grep -v "^/usr/lib" | sort -u)
    
    for dep in $deps; do
        if [ -f "$dep" ] && [ ! -f "$dest_dir/$(basename "$dep")" ]; then
            log "Copying dependency: $(basename "$dep")"
            cp "$dep" "$dest_dir/"
        fi
    done
}

copy_deps "$APPDIR/usr/bin/vivictpp" "$APPDIR/usr/lib"

# Create desktop file
log "Creating desktop file..."
cat > "$APPDIR/usr/share/applications/vivictpp.desktop" << EOF
[Desktop Entry]
Type=Application
Name=Vivict++
Comment=Video comparison tool for subjective quality assessment
GenericName=Video Comparison Tool
Exec=vivictpp
Icon=vivictpp
StartupNotify=true
NoDisplay=false
Categories=AudioVideo;Video;
StartupWMClass=vivictpp
MimeType=video/mp4;video/x-msvideo;video/quicktime;video/x-matroska;video/webm;
Keywords=video;comparison;quality;encoding;
EOF

# Create a simple icon (if one doesn't exist in the repo)
log "Creating application icon..."
cat > "$APPDIR/usr/share/icons/hicolor/scalable/apps/vivictpp.svg" << 'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<svg width="64" height="64" viewBox="0 0 64 64" xmlns="http://www.w3.org/2000/svg">
  <rect width="64" height="64" rx="8" fill="#2c3e50"/>
  <rect x="8" y="16" width="20" height="32" rx="2" fill="#3498db"/>
  <rect x="36" y="16" width="20" height="32" rx="2" fill="#e74c3c"/>
  <polygon points="32,10 36,18 28,18" fill="#f39c12"/>
  <text x="32" y="58" text-anchor="middle" fill="white" font-family="Arial" font-size="8">V++</text>
</svg>
EOF

# Copy the icon to pixmaps as well for broader compatibility
cp "$APPDIR/usr/share/icons/hicolor/scalable/apps/vivictpp.svg" "$APPDIR/usr/share/pixmaps/"

# Create AppRun script
log "Creating AppRun script..."
cat > "$APPDIR/AppRun" << 'EOF'
#!/bin/bash

# AppRun script for vivictpp

SELF=$(readlink -f "$0")
HERE=${SELF%/*}

# Set library path to include bundled libraries
export LD_LIBRARY_PATH="${HERE}/usr/lib:${LD_LIBRARY_PATH}"

# Set path to find the binary
export PATH="${HERE}/usr/bin:${PATH}"

# Try to find the vivictpp binary
BINARY="${HERE}/usr/bin/vivictpp"

if [ ! -x "$BINARY" ]; then
    echo "Error: vivictpp binary not found at $BINARY"
    exit 1
fi

# Execute the application
exec "$BINARY" "$@"
EOF

chmod +x "$APPDIR/AppRun"

# Copy desktop file to root of AppDir and make it the main one
cp "$APPDIR/usr/share/applications/vivictpp.desktop" "$APPDIR/"

# Create the AppImage
log "Creating AppImage..."
VERSION="$VERSION" $APPIMAGETOOL "$APPDIR" "${APPNAME}-${VERSION}-${ARCH}.AppImage"

if [ $? -eq 0 ]; then
    log "AppImage created successfully: ${APPNAME}-${VERSION}-${ARCH}.AppImage"
    log "File size: $(du -h "${APPNAME}-${VERSION}-${ARCH}.AppImage" | cut -f1)"
    log "You can now run: ./${APPNAME}-${VERSION}-${ARCH}.AppImage"
else
    error "Failed to create AppImage"
fi

# Make the AppImage executable
chmod +x "${APPNAME}-${VERSION}-${ARCH}.AppImage"

log "Build complete!"
