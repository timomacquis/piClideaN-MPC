# Credits, provenance and licenses

## Original work

piClideaN MIDI engine, native UI generator, parameter model and packaging:
Copyright 2026 Timo, MIT (LICENSE). Euclidean generation is implemented locally
as a deterministic evenly-spaced binary pattern. No proprietary firmware/DSP,
Euclidier binary, preset bank or runtime component is incorporated.

## sd88me MPC VST wrapper

[sd88me/mpc-vst-plugins](https://github.com/sd88me/mpc-vst-plugins), MIT,
Copyright 2026 sd88me. Thanks for the wrapper, generator, native skin conventions,
MIDI timing guidance and the October Q-Link fixes. Pinned revision
97a2effca4114dd00f738b018e2f0d1d3ccb2d7f. Complete license, original input hashes,
patched hash and full local patch: third_party/mpc-vst-wrapper/.
The wrapper is rebuilt into this candidate; other installed plugins are untouched.

## Native skin reference

reference/skin-base.zip is a local piLocKsteP native schema reference. Its SHA-256
is checked before use. Only the TUI/Q-Link schema is read; original components
and pages are replaced. No old plugin image or runtime is included in the release.
The sd88me Force Shadow reference license is in licenses/FORCE-SHADOW-LICENSE.
Our PNG artwork is generated locally. MPC and its built-in controls remain Akai's.

## Typography

Titillium Web, SIL Open Font License 1.1. Fonts and OFL are in reference/fonts/.
Used by the local artwork generator. Native text refers to the existing MPC font.

## ALSA and compiler

ALSA development headers are included for reproducible compilation; see
licenses/ALSA-copyright. Link against the target's system libasound.so.2; no ALSA
runtime library is copied into the plugin package. The compiler is not part of
release/source ZIPs: official Debian downloads are pinned in docs/TOOLCHAIN.json.
See licenses/GCC-copyright and each downloaded package's own license notices.

## Musical and target references

- [Brugio96/euclidean-sequencer](https://github.com/Brugio96/euclidean-sequencer):
  musical workflow and named rhythm inspiration; no code/runtime dependency.
- [sd88me/mpc-vst-euclidier](https://github.com/sd88me/mpc-vst-euclidier):
  comparison/reference only. This is an independent new plugin.
- [saustin2010/mpc-vst-incipience](https://github.com/saustin2010/mpc-vst-incipience):
  documented drum note profile. Notes remain user-editable and are not inferred
  from an installed Incipience instance. See docs/TARGET-INCIPIENCE.md.
- The local decoded empty Force Drum project supplied the reference pad/note
  table, with its source hash in reference/DRUMTRACK-REFERENCE.json. No project,
  sample, personal settings or recording is shipped.

## Extended modulation generators

Random Walk, Random Loop, Bounce and cross-LFO control are original MIT implementations. The documented behaviours of [Lunar Modulator](https://github.com/ip2k/lunar-modulator) informed the design; no Lunar audio engine, firmware or source implementation is incorporated.

## Groove MIDI Dataset statistics

The guided rhythmic generator includes compact derived GMD v1.0.0 statistics under CC BY 4.0. Attribution, source identity, modifications and reproduction: [licenses/GROOVE-ATTRIBUTION.md](licenses/GROOVE-ATTRIBUTION.md). No raw MIDI/audio or GrooVAE runtime is embedded.
