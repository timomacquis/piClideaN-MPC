#!/usr/bin/env bash
set -eu
cd "$(dirname "$0")/.."
: "${PICN_GROOVE_PYTHON:=python3}"
"$PICN_GROOVE_PYTHON" tests/groove_statistics_test.py
cc -std=c11 -D_POSIX_C_SOURCE=200809L -D_GNU_SOURCE -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -Ibuild -Ithird_party/mpc-vst-wrapper/wrapper -Ithird_party/alsa-dev/usr/include tests/groove_runtime_test.c -l:libasound.so.2 -lm -lpthread -ldl -o build/groove-runtime-test
ASAN_OPTIONS=detect_leaks=1 ./build/groove-runtime-test
bash scripts/test_variations134.sh
