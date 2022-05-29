#!/bin/sh

set -e

emcmake cmake -B build-web . \
  -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release

cmake --build build-web

python -m http.server --bind 127.0.0.1 --directory build-web
