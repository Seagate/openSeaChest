#!/bin/sh

# The purpose of this script is to execute the json-c project's cmake build for the makefiles
# When this was attempted in the makefiles themselves, it failed with bizarre errors.
# TODO: Check arguments

cmake -S ../../subprojects/json-c/ -B ../../subprojects/json-c/build -DBUILD_SHARED_LIBS=OFF -DDISABLE_EXTRA_LIBS=ON -DBUILD_TESTING=OFF -DDISABLE_BSYMBOLIC=ON -DCMAKE_BUILD_TYPE=Release
cmake --build ../../subprojects/json-c/build
