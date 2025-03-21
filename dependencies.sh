#!/bin/bash
# dependencies.sh: Install system dependencies for QML Client-Server Suite on Ubuntu/WSL
echo "Updating package lists..."
sudo apt-get update

echo "Installing system dependencies..."
sudo apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    git \
    qt6-base-dev \
    qt6-declarative-dev \
    qt6-tools-dev \
    qt6-tools-dev-tools \
    qml6-module-qtquick-controls \
    qml6-module-qtquick-layouts \
    qml6-module-qtqml-workerscript \
    libxcb-cursor0 \
    libxcb-cursor-dev \
    libxcb-image0-dev \
    libxcb-render-util0-dev \
    libxcb-render0-dev \
    libxcb-shm0-dev \
    python3 \
    python3-dev \
    python3-pip \
    python3-flask \
    python3-pytest \
    usbutils

echo "Cleaning up package cache..."
sudo rm -rf /var/lib/apt/lists/*

echo "All dependencies have been installed successfully."
