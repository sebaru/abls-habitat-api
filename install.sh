#!/bin/bash

# Install script for ABLS Habitat API
# This script installs the project from the 'build' directory using CMake

set -e  # Exit on error

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build"

echo "Installing ABLS Habitat API..."
echo "Project directory: $PROJECT_DIR"
echo "Build directory: $BUILD_DIR"
echo "Number of processors: $(nproc)"

# Create build directory if it doesn't exist
if [ ! -d "$BUILD_DIR" ]; then
    echo "build directory does not exist. Stopping"
    exit 1
fi

# Navigate to build directory
cd "$BUILD_DIR"

# Install the project
echo "Installing project..."
sudo cmake --install .
sudo systemctl daemon-reload

echo ""
echo "Please edit /etc/abls-habitat-api.conf before starting"
echo "You can start the service with: sudo systemctl start abls-habitat-api"
