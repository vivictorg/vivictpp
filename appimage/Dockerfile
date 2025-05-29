# Dockerfile for building vivictpp AppImage
# This provides a clean, reproducible build environment

FROM ubuntu:20.04

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=UTC

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    curl \
    pkg-config \
    python3 \
    python3-pip \
    python3-setuptools \
    python3-wheel \
    ninja-build \
    meson \
    libfreetype6-dev \
    libavformat-dev \
    libavcodec-dev \
    libavfilter-dev \
    libswscale-dev \
    libsdl2-dev \
    libsdl2-ttf-dev \
    file \
    desktop-file-utils \
    && rm -rf /var/lib/apt/lists/*

# Create a non-root user for building
RUN useradd -m -s /bin/bash builder
USER builder
WORKDIR /home/builder

# Download appimagetool
RUN wget -O appimagetool "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage" && \
    chmod +x appimagetool

# Copy build script
COPY --chown=builder:builder build_appimage.sh .
RUN chmod +x build_appimage.sh

# Clone the repository
RUN git clone https://github.com/vivictorg/vivictpp.git

# Set the build script as the entry point
ENTRYPOINT ["./build_appimage.sh"]
