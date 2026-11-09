#!/usr/bin/env bash
set -eu
cd "$(dirname "$0")/.."
bash scripts/test_mod.sh
for test in release131_test mod_preview_worker_test next_features_test ui7_engine_test scales_panel_test mod_refresh_wrapper_test; do
 cc -std=c11 -D_POSIX_C_SOURCE=200809L -D_GNU_SOURCE -DPICN_NOTIFICATION_BUDGET=8 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -Ibuild -Ithird_party/mpc-vst-wrapper/wrapper -Ithird_party/alsa-dev/usr/include "tests/$test.c" -l:libasound.so.2 -lm -lpthread -ldl -o "build/$test"
 ASAN_OPTIONS=detect_leaks=1 "build/$test"
done
