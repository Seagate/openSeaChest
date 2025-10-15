#!/bin/sh

# The purpose of this script is to execute the json-c project's cmake build for the makefiles
# When this was attempted in the makefiles themselves, it failed with bizarre errors.
# TODO: Check arguments

cmake -S ../../../json-c/ -B ../../../json-c/build \
    -DBUILD_TESTING=OFF \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=OFF \
    -DDISABLE_EXTRA_LIBS=ON \
    -DDISABLE_BSYMBOLIC=ON \
    -DBUILD_APPS=OFF \
    -DCMAKE_POLICY_DEFAULT_CMP0126=NEW \
    -DCMAKE_POLICY_DEFAULT_CMP0156=NEW \
    -DCMAKE_POLICY_DEFAULT_CMP0179=NEW
cmake --build ../../../json-c/build
