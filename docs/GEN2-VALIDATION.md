# Gen2 1.3.0 integration checks

Date: 2026-11-01. Based on Thomas Phillips’s PR #2, commits ec24e397 through 5b9ee64.

## Offline results

- Current 1.3.0 engine and pinned wrapper compiled for ELF64 AArch64 with Debian Bookworm GCC 12.
- No changes to engine sources, wrapper, parameter definitions or parameter indices.
- Required glibc symbol versions: 2.17, 2.29 and 2.34.
- ZIP CRC and all payload SHA-256 entries verified.
- Manifest architecture, version and packaged engine checksum verified.
- All 1,319 native Image references have bitmap area at least 6,400 pixels in the Gen2 skin.
- The default Gen1 skin generator was run before and after integration using the same Python/Pillow environment: every generated file is byte-identical.
- The existing published Gen1 installation ZIP still matches its original checksum.
- Build and package scripts pass Python syntax checks.

Engine SHA-256: `4f1e73761dcd250f7fc2882c1533e6438287d889b71fcffc62af8e0b3a260f94`.

## Device status

Thomas reported successful loading and MIDI playback of his original 1.2.4 build on MPC Live III, with the image-padding correction.

This rebuilt 1.3.0 package has not yet been tested on a Gen2 device. Installation/removal, all pages, touch interaction, MIDI playback, project save/reload and CPU/audio behavior remain to be checked there. Other Gen2 models are untested. No new hardware validation is claimed.

Gen1/Force remains the existing release; this integration does not install anything on a device.
