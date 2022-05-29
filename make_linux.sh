#!/bin/sh

set -e

cmake -B build-linux . \
  -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug

cmake --build build-linux

