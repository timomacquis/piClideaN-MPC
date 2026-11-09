# Build — base de travail correspondant à 1.2.0

## Inputs

The project is self-contained: original C11 engine, pinned wrapper/generator,
ALSA headers, font inputs and reference native skin schema are included. No
installed Euclidier, piLocKsteP or pForceStepper is required.

Host: Linux x86-64 / WSL with Python 3.10+, dpkg-deb and network access for the
initial compiler bootstrap. The ARM toolchain is extracted inside this project;
it does not replace the host compiler and requires no sudo.

```sh
python3 scripts/bootstrap-toolchain.py
# Use a Python interpreter with Pillow for native skin images.
# On Linux, after making Pillow available in your chosen environment:
export PICN_SKIN_PYTHON=python3
python3 scripts/build.py
python3 scripts/package.py
```

On this Windows workstation, build.py defaults to the existing bundled Windows
Python with Pillow. Elsewhere, explicitly select your Pillow-enabled interpreter
with PICN_SKIN_PYTHON. The engine is cross-compiled by the local Debian ARMv7 GCC
12 toolchain. `docs/TOOLCHAIN.json` pins every downloaded package and SHA-256.
A missing archived upstream Debian package must be recovered with its exact hash;
do not silently substitute a newer compiler or library.

`build.py` verifies vendored input hashes, regenerates parameters, enforces
append-only `params.lock`, compiles ARM hard-float, strips the plugin, generates
five native pages, and records ELF requirements in Validation/BUILD.json.
`package.py` assembles the current engine and skin, checks archive CRCs and hashes,
and creates separate installation and source ZIPs. Neither script contacts an
instrument, changes MPC.settings, installs a plugin or flashes firmware.

## Wrapper

Pinned commit: 97a2effca4114dd00f738b018e2f0d1d3ccb2d7f of sd88me/mpc-vst-plugins.
The shipped wrapper already includes our changes; prepare_wrapper.py verifies it.
LOCAL-CONTEXT.patch documents the complete diff against that commit:

- Host sample rate and cycle bounds passed together under the engine mutex.
- Invalid host timing fails silent; suspension releases owned notes.
- Only canonical lane/routing parameters are advertised for automation.
- ARM pthread attributes bind to their existing GLIBC_2.4 ABI instead of requiring
  the GLIBC_2.34 default versions chosen by the newer cross-linker.

The engine requires GLIBC_2.17 at most and ALSA_0.9 APIs. This symbol
check is not hardware qualification. System libasound/libc/libm remain required.
No private copy of a system shared library is shipped.

## Parameter compatibility

2196 keys, 1427 canonical automatable parameters. The initial 539 indices are retained. Append parameters at the end.
Do not delete/reorder params.lock or use GEN_VST_RELOCK. Contextual PATTERN knobs
edit the selected line; Track Edit automation exposes each line's actual fields
so changing the selected line cannot redirect recorded automation. Physical
Q-Link behavior on both screens still requires hardware qualification.

## Qualification

This build request did not include running implementation tests. No engine
self-tests, upstream test_port suite, MIDI capture or hardware test was run.
Before calling a release validated, execute the qualification checklist and
request separate authorization before installing/restarting MPC.

## Initial deployment preparation adjustment

The first preparation on Force rejected the optional -fstack-protector-strong
build because its handle-specific loader ABI probe did not resolve
__stack_chk_guard@GLIBC_2.4. Read-only inspection of the actual loader confirms
that the symbol is exported; this is not evidence that the system lacks it.
The candidate now uses the existing Force ports' baseline compiler options
without adding that stack-protector option. RELRO/NOW/NX and input bounds remain.
No installer ABI checks are relaxed. This removes the newly introduced canary
instrumentation; it is a compile-option adjustment, not a change to MIDI or UI.
The rejected preparation did not stop MPC or alter registered plugins/settings.

After deployment, refresh documentation/source snapshots with
`python3 scripts/package.py --sources-only`. This preserves the exact installable
ZIP and its identity in the device installation receipt.

Candidate.2 identity/interaction changes: CANDIDATE-2-CORRECTIONS.md.

## Build receipts — candidate.5

The build stores SHA-256 values for source, vendored, parameter and skin-generation inputs and generated skin outputs. Packaging requires the same VERSION, the same input hashes and the same skin files. Rebuild after a functional source edit. Documentation-only source refresh keeps the frozen installation ZIP unchanged and verifies its existing identity. SOURCE_DATE_EPOCH supplies a reproducible UTC timestamp; report dates use Europe/Paris.

HAS_DISPLAY_REV is disabled for this port: bounded engine notifications drive numeric visual changes, targeting roughly 30 Hz. The pinned wrapper retains the Q-Link fix; LOCAL-NOTIFICATIONS.patch changes only parameter-name lookup to binary search using the generated sorted index table. This is not a measured frame-rate or CPU guarantee.

## Evolution build details

After build.py, generate the offline layouts using the Pillow interpreter and scripts/preview_skin.py, then run python3 scripts/build_manual.py and python3 scripts/package.py. The manual embeds the four offline layout images. No engine tests are run.

PICN2 remains inside the wrapper’s 8192-byte chunk. Per-step canonical parameters use -1 for inheritance; eight contextual Q-Link aliases are not advertised for automation. The sorted lookup avoids scanning all 2196 names for each engine notification. The build also checks that inputs did not change between compilation and skin receipt creation.

## Audit candidate.2

Continuous Q-Link aliases keep fractional input, settle it against the real target range and retain direction across successive writes. Native fields keep real ranges. New momentary toggle commands keep their hit targets stable. Read-only step filmstrips replace conditional state components. The package is verified in a staging file before atomic replacement. See AUDIT-020-CANDIDATE2-20261023.md for scope and limits.

## Sixteen-lane model

3,925 parameters; old 2,196 definitions preserved, 1,729 appended. PICN3 chunk storage: 32 KiB via port-local MPC_CHUNK_BYTES define; documented LOCAL-STATE-CAPACITY.patch. Native skin has five pages. No engine runtime or device tests implied by compilation/packaging.

## GATE / HOLD — 1.1.1-candidate.1

3941 parameters: exact 3925-definition prefix retained; sixteen HOLD flags appended. PICN4 appends sixteen flags to the PICN3 payload; older chunks read with GATE defaults. Existing 32 KiB chunk storage remains. No additional MIDI port, worker, timer or runtime allocation is introduced. Qualification scope: docs/EVOLUTION-GATE-HOLD-AMBRE.md.
