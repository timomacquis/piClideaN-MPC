#!/usr/bin/env bash
# πClideaN 1.4.0 : import de motifs MicroTonic (.mtpreset).
#   1. lecteur de motifs (tests unitaires) ;
#   2. comparaison au lecteur du projet FM-1 (f-ToniC) sur les kits privés, si node, le lecteur et les kits sont présents ;
#   3. import dans le moteur (navigateur, fil de lecture, choix A-L / paires, lignes 1-8, état).
set -eu
cd "$(dirname "$0")/.."
mkdir -p build
cc -std=c11 -D_POSIX_C_SOURCE=200809L -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer tests/mtpreset_sequence_test.c -lm -o build/mtpreset_sequence_test
ASAN_OPTIONS=detect_leaks=1 ./build/mtpreset_sequence_test
if command -v node >/dev/null 2>&1; then
 ASAN_OPTIONS=detect_leaks=0 node tests/mtpreset_sequence_compare.mjs build/mtpreset_sequence_test "${FM1_READER:-../../FM-1-Dev/f-ToniC/manager/mt-preset.mjs}" "${MT_FIXTURES:-../πToniC/Private/mtpreset-tests}"
else echo "SKIP node absent : comparaison au lecteur FM-1 non faite"; fi
cc -std=c11 -D_POSIX_C_SOURCE=200809L -D_GNU_SOURCE -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -Ibuild -Ithird_party/mpc-vst-wrapper/wrapper -Ithird_party/alsa-dev/usr/include tests/mtpreset_import_test.c -l:libasound.so.2 -lm -lpthread -ldl -o build/mtpreset_import_test
ASAN_OPTIONS=detect_leaks=1 ./build/mtpreset_import_test
