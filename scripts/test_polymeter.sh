#!/usr/bin/env bash
set -eu
cd "$(dirname "$0")/.."
mkdir -p build
cc -std=c11 -D_POSIX_C_SOURCE=200809L -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -Ithird_party/mpc-vst-wrapper/wrapper -Ithird_party/alsa-dev/usr/include tests/polymeter_loop_test.c -l:libasound.so.2 -lm -lpthread -o build/polymeter_loop_test
ASAN_OPTIONS=detect_leaks=1 ./build/polymeter_loop_test
