#!/usr/bin/env bash
set -eu
cd "$(dirname "$0")/.."
python3 scripts/generate_factory_programs.py
cc -std=c11 -D_POSIX_C_SOURCE=200809L -D_GNU_SOURCE -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -Ibuild -Ithird_party/mpc-vst-wrapper/wrapper -Ithird_party/alsa-dev/usr/include tests/native_programs_test.c -l:libasound.so.2 -lm -lpthread -ldl -o build/native_programs_test
ASAN_OPTIONS=detect_leaks=1 ./build/native_programs_test
