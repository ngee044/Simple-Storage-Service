#!/bin/bash

rm -rf build

mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE="../../vcpkg/scripts/buildsystems/vcpkg.cmake" -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF

cmake --build . --config Release

echo "Build finished. Executables are in build/out/"
