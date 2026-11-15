# Installation and removal (Gen2, aarch64)

The standard package `ClideaN-MPC-1.3.7-mpc-aarch64.zip` includes generated instructions in `INSTALL.md`; this page is the detailed Gen2 guide. For Gen1 and Force, use `ClideaN-MPC-1.3.7-mpc-armv7.zip` and its own guide.

## Requirements

- Akai MPC Gen2 (MPC Live III; other models require separate verification): a 64-bit aarch64 userland. Check with `uname -m`, which must print `aarch64`.
- Root shell access (SSH) or a compatible installer that handles aarch64 portable plugin ZIPs and the `pluginList-arm-64bit` registration.
- A writable, executable plugin folder. On OS versions with the official VST2 folder loader, use the device’s `VST2 Plugins` location and the included `plugin.json` (`file64`). Older supported hosts use `Synths`. Removable drives are often mounted `noexec` and cannot load a plugin. On an MPC Live III the internal `/nvme/Synths` is writable and executable; `/synths` is read-only.

Thomas Phillips tested the original 1.2.4 port on an MPC Live III (aarch64, az0x base 5.0.17). This rebuilt 1.3.7 package is pending a fresh device test. Other Gen2 models are untested.

## Install with the standard package

1. Save the project and back up the current plugin/settings.
2. Extract `ClideaN-MPC-1.3.7-mpc-aarch64.zip` and read its generated `INSTALL.md` inside `ClideaN-1.3.7/`.
3. Use a compatible installer, or copy that complete folder to the device and run its included `install.sh` with the desired executable Synths destination. The script stops and restarts MPC: obtain authorization after saving first.
4. The installed content folder is **`Timo - VST - πClideaN`**, containing `piclidean_mpc.so`. Registration is generated from the package metadata; do not reuse an old handwritten XML entry from 1.3.1.
5. Open **πClideaN**, enable its corresponding `piClideaN MPC Out` MIDI input on the target track, and match the notes/channel.

Use the package's removal instructions for uninstalling; registration and files must be handled together. Projects and personal samples remain separate. The exact ZIP's generated metadata takes precedence over loose historical `plugin-meta.xml` files in the development tree.

## Routing checklist

- πClideaN produces MIDI, not audio: its own audio output is silent.
- Enable its output on the intended target track, using the same MIDI channel.
- Match the MIDI note in CONFIG to the target pad or sound.
- Disable unwanted parallel MIDI inputs to avoid duplicate hits.
- For multiple instances, choose the corresponding numbered output port.
- In HOLD, the target must respond to Note Off; One Shot playback may ignore it.

## Gen2 skin note

MPC Live III draws a non-button `Image` component as a blank box when its bitmap is smaller than about 5400 pixels in area. The Gen2 package therefore ships a skin whose small images are padded to at least 6400 pixels (MPC clips them to their bounds). This is done at build time with `PICN_SKIN_MIN_IMAGE_AREA`; the Gen1 skin is unchanged.
