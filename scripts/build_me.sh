#!/bin/bash

set -e  # stop on error

# Get script location
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

BUILD_DIR="$PROJECT_ROOT/build"
RUN_TESTS=false

# Parse args
for arg in "$@"; do
    case "$arg" in
        clean)
            rm -rf "$BUILD_DIR"
            echo "Build directory cleaned."
            ;;
        test)
            RUN_TESTS=true
            ;;
        *)
            echo "Unknown option: $arg"
            echo "Usage: $0 [clean] [test]"
            exit 1
            ;;
    esac
done

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure
cmake -DCMAKE_BUILD_TYPE=Release ..

# Build
cmake --build .

# Optionally run tests
if [ "$RUN_TESTS" = true ]; then
    cmake --build . --target run_tests
fi

exit 0
