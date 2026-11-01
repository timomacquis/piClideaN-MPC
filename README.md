# πClideaN 16 MPC

**Version 1.3.0 · Sixteen-lane hybrid MIDI sequencer for Akai MPC OS**

![πClideaN 16 MPC](pattern-line-force-1.2.5.png)

Build evolving rhythmic and melodic patterns with sixteen independent lanes, per-step expression, scales and a native touch interface. Drive synthesizers, drum instruments and sample tracks through the plugin’s dedicated MIDI output.

## Features

- Scale and mode assignment across sixteen lanes, with a selectable root and register.
- Rd ALL generates sixteen fresh patterns while preserving mute states and pitches.
- Native choice panels for modulation settings, scales and playback direction.


- Four independent modulation slots, each with two LFOs and a bipolar MIX control.
- Eleven waveform/generator choices, with cross-LFO rate, depth and offset modulation.
- Cached background waveform previews and streamlined track graphics.
- Static four-bar waveform previews, configurable destinations and threshold-based Note On.
- One CONFIG page for all sixteen MIDI notes and GATE / HOLD modes.

- Sixteen lanes with 1–32 steps each.
- Euclidean Steps, Hits and Rotate controls, with five rhythmic divisions.
- Selective randomization of lane parameters with a two-stage RAND control.
- Per-lane velocity, probability, velocity humanization and gate length.
- Per-step velocity, probability, ratchet and ratchet decay.
- Forward, reverse, two pendulum modes and shuffled traversal.
- Automatic pattern rotation at a configurable cycle interval.
- Per-lane GATE / HOLD for short hits and sustained notes.
- Touch selection, note drawing, contextual erase, mute and solo.
- Rounded controls and inset circular status indicators across the plugin pages.
- Read-only LINE references in EDIT, with per-step controls kept active.
- Dimmed lane selectors for explicitly muted lanes, preserving selection and activity feedback.
- Azure activity pulses on all sixteen lane buttons during playback.
- Eight contextual Q-Link controls and native project state storage.
- Independent MIDI client, output port and scheduling queue for each instance.

The plugin generates MIDI only. Use a separate target instrument or sample track to produce sound.

## Screenshots

### MOD — dual LFO modulation

![MOD 1 assigned to A16: Random Smooth, Bounce and LFO chaining](docs/images/mod-force-1.3.0.png)

### Tracks — pattern overview and playback controls

![TR 01–08 on Akai Force: rounded controls, step indicators and automatic rotation](docs/images/tracks-force-1.2.5.png)

### CONFIG — MIDI notes and GATE / HOLD

![CONFIG on Akai Force: sixteen MIDI notes and independent GATE / HOLD controls](docs/images/config-force-1.2.5.png)

## Download and setup

Choose the package for your device. Both builds use πClideaN **1.3.0**.

| Device generation | Architecture | Download | Installation |
| --- | --- | --- | --- |
| **Gen1 / Force** | **ARM32** — ARMv7 | [Gen1 / ARM32 ZIP](https://github.com/timomacquis/piClideaN16-MPC/releases/download/v1.3.0/piClideaN-MPC-1.3.0-armv7.zip) | [Gen1 / Force guide](docs/INSTALLATION.md) |
| **Gen2** | **ARM64** — AArch64 | [Gen2 / ARM64 ZIP](https://github.com/timomacquis/piClideaN16-MPC/releases/download/v1.3.0/piClideaN-MPC-1.3.0-aarch64.zip) | [Gen2 guide](docs/INSTALLATION-GEN2.md) |

Gen1 / Force is the existing device-validated release and is unchanged. The Gen2 1.3.0 build passes offline checks and awaits device validation. Thomas Phillips tested the original 1.2.4 port on MPC Live III; other Gen2 models are untested. Thanks to Thomas for the port and Live III skin fix. See [Gen2 validation](docs/GEN2-VALIDATION.md) and [build instructions](docs/BUILD-GEN2.md).

## Documentation

- [User manual](docs/USER-MANUAL.md) · [Standalone illustrated manual](docs/USER-MANUAL.html)
- [MIDI engine](docs/ENGINE.md)
- [Implementation](docs/IMPLEMENTATION.md)
- [Build from source](docs/BUILD.md)
- [Third-party notices](THIRD_PARTY.md)

## Repository layout

```text
Installation/       Installable ARMv7 ZIP and checksum
src/                MIDI engine
scripts/            Parameter, skin, build and package tools
docs/               User and developer documentation
reference/          Skin schema, note map and font inputs
third_party/        Pinned wrapper, generator and ALSA headers
licenses/           Dependency license notices
params.json         Parameter definitions
params.lock         Fixed parameter indices
vst.json            Plugin build configuration
```

## Requirements

Akai Gen1 (Tested on Akai FORCE) running an OS environment that supports community ARMv7 VST2 plugins and native plugin skins. This is not a stock firmware add-on installer. SSD installation requires executable access to the SSD's `Synths` directory. The package uses regular files and requires no symbolic links.

## License

MIT. See [LICENSE](LICENSE) and [THIRD_PARTY.md](THIRD_PARTY.md).
