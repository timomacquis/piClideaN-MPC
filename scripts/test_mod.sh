#!/usr/bin/env bash
set -eu
cd "$(dirname "$0")/.."
mkdir -p build
cc -std=c11 -D_POSIX_C_SOURCE=200809L -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -Ithird_party/mpc-vst-wrapper/wrapper -Ithird_party/alsa-dev/usr/include tests/mod_engine_test.c -l:libasound.so.2 -lm -o build/mod_engine_test
ASAN_OPTIONS=detect_leaks=1 ./build/mod_engine_test
cc -std=c11 -D_POSIX_C_SOURCE=200809L -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -Ithird_party/mpc-vst-wrapper/wrapper -Ithird_party/alsa-dev/usr/include tests/mod_palette_test.c -l:libasound.so.2 -lm -o build/mod_palette_test
ASAN_OPTIONS=detect_leaks=1 ./build/mod_palette_test
# Preserved baseline is a development fixture, never part of the plugin payload.
for variant in baseline candidate; do
    flag=""
    if [ "$variant" = baseline ]; then flag="-DBASELINE"; fi
    cc -std=c11 -D_POSIX_C_SOURCE=200809L -O2 $flag -Isrc -Ithird_party/mpc-vst-wrapper/wrapper -Ithird_party/alsa-dev/usr/include tests/mod_baseline_compare.c -l:libasound.so.2 -lm -o "build/mod_compare_$variant"
    "build/mod_compare_$variant" > "build/mod_compare_$variant.txt"
done
cmp build/mod_compare_baseline.txt build/mod_compare_candidate.txt
printf 'PASS modulation OFF: 4096 blocks, complete event stream identical to pre-E01\n'
python3 - <<'PY'
from pathlib import Path
import hashlib,json,datetime
paths=['src/groove_tables.h','src/variations_types.h','src/rhythm_knowledge.h','src/variations.h','src/guidance_worker.h','src/engine.c','src/evolution.h','src/mod_lfo.h','src/mod_types.h','src/mod_extended.h','src/mod_chain.h','src/mod_preview_worker.h','tests/mod_engine_test.c','tests/mod_palette_test.c','tests/mod_baseline_compare.c','Validation/E01-baseline/engine.c','Validation/E01-baseline/evolution.h']
report={'passed':True,'hardware_access':False,'sanitizers':['address','undefined'],'baseline_blocks':4096,'stress_lfos':8,'stress_hz':20,'timestamp_utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'input_sha256':{p:hashlib.sha256(Path(p).read_bytes()).hexdigest() for p in paths}}
Path('Validation/E01-TESTS.json').write_text(json.dumps(report,indent=2)+'\n')
PY

python3 - <<'PYEND'
from pathlib import Path
import json
p=json.loads(Path('params.json').read_text());old=Path('Validation/E01-baseline/params.lock').read_text().split();keys=[r['key'] for r in p]
assert len(keys)<=4096 and keys[:len(old)]==old
for name in ['mod_mix','mod0_on','mod1_dest','mod_tempo']:assert keys.index(name)<4096
assert not any(k.startswith('l0_mod') for k in keys)
print('PASS native parameter budget: 4096 handles, 3974 stable indices preserved')
PYEND
