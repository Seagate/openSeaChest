#!/bin/sh

# Usage: ./build_jsonc.sh <build_dir> [<cc>]
# Example: ./build_jsonc.sh build gcc "Unix Makefiles"

set -e

# Arguments
build_dir="$1"
cc="${2:-gcc}"  # Default to gcc if not provided

# Resolve full paths
jsonc_src=$(cd "$(dirname "$0")/../subprojects/json-c" && pwd)
jsonc_build="$jsonc_src/$build_dir"

# Ensure build directory exists
mkdir -p "$jsonc_build"

# Set compiler
export CC="$cc"

# Run CMake
cmake -S "$jsonc_src" -B "$jsonc_build" \
    -DBUILD_TESTING=OFF \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=OFF \
    -DDISABLE_EXTRA_LIBS=ON \
    -DDISABLE_BSYMBOLIC=ON \
    -DBUILD_APPS=OFF \
    -DCMAKE_POLICY_DEFAULT_CMP0126=NEW \
    -DCMAKE_POLICY_DEFAULT_CMP0156=NEW \
    -DCMAKE_POLICY_DEFAULT_CMP0179=NEW \
    -DCMAKE_INSTALL_PREFIX="$jsonc_build/__no_install__" \
    -DCMAKE_C_COMPILER="$cc"

# Build
cmake --build "$jsonc_build"
