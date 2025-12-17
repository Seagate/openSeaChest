#!/bin/sh

# Usage: ./build_jsonc.sh <build_dir> [<cc>]
# Example: ./build_jsonc.sh build gcc "Unix Makefiles"

set -e

# Arguments
build_dir="$1"
cc="${2:-gcc}"  # Default to gcc if not provided

# Detect Windows runners (GitHub Actions uses mingw path on Windows)
case "$OS$PROCESSOR_ARCHITECTURE" in
  *Windows*|*windows*|*MINGW*|*MSYS*)
    generator="MinGW Makefiles"
    make_program="$(command -v mingw32-make || true)"
    # fallback to msys/mingw make if present
    [ -z "$make_program" ] && make_program="$(command -v make || true)"
    ;;
  *)
    generator=""
    make_program=""
    ;;
esac

echo $generator
echo $make_program

# Resolve full paths
jsonc_src=$(cd "$(dirname "$0")/../subprojects/json-c" && pwd)
jsonc_build="$jsonc_src/$build_dir"

# Ensure build directory exists
mkdir -p "$jsonc_build"
echo "Current user:" && whoami
echo "Build directory permissions:"
ls -ld "$jsonc_build"

# Set compiler
export CC="$cc"

# Run CMake
cmake -S "$jsonc_src" -B "$jsonc_build" \
    ${generator:+-G "$generator"} \
    ${make_program:+-DCMAKE_MAKE_PROGRAM="$make_program"} \
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
    -DCMAKE_C_COMPILER="$cc" \
    -DDISABLE_WERROR=ON \
    -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY \
    -DCMAKE_DISABLE_SOURCE_CHANGES=ON \
    -DCMAKE_DISABLE_IN_SOURCE_BUILD=ON \
    -DCMAKE_VERBOSE_MAKEFILE=ON

# Build
cmake --build "$jsonc_build"
