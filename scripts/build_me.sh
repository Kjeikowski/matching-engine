#!/bin/bash

set -e  # stop on error


# Get script location
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

BUILD_DIR="$PROJECT_ROOT/build"


if [ "${1:-}" = "clean" ]; then
    rm -rf "$BUILD_DIR"
    echo "Build directory cleaned."
    exit 0
fi


# Create build directory safely
mkdir -p "$BUILD_DIR"

cd "$BUILD_DIR"

cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
