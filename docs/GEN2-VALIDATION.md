# Gen2 1.3.2 — validation status

Date: 2026-11-06. The AArch64 integration retains Thomas Phillips’s PR #2 contribution and its Gen2-only small Image-component workaround.

## Release checks

The same 1.3.2 engine sources, pinned wrapper and stable parameter table are built for Gen1 ARMv7 and Gen2 AArch64. Build receipts verify source and skin hashes; package checks verify ELF class/machine, checksums and archive contents. The Gen2 skin pads small Image assets to at least 6,400 pixels while keeping artwork at the top left. Gen1 padding remains disabled.

Release build results and checksums accompany the published packages. Runtime performance and audio/MIDI on Gen2 cannot be established by cross-compilation or offline tests.

## Device status

This 1.3.2 Gen2 package has not been tested on a Gen2 device. Thomas reported loading and MIDI playback of the original 1.2.4 contribution on MPC Live III. That report is not evidence for the current release or other Gen2 models.

Installation/removal, all pages and small-image rendering, touch/Q-Links, MIDI timing, count-in, sustained GATE/HOLD, project save/reload and CPU/audio behavior remain to be checked on Gen2. No device was modified to prepare this release.
