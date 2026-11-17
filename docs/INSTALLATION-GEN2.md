# Gen2 ARM64 — πClideaN 1.3.8

Compiled for 64-bit Gen2 MPC devices (`uname -m` prints `aarch64`). **Not tested on Gen2 hardware.**

Install exactly as on Gen1 (see [Installation and removal](INSTALLATION.md)): unzip `piClideaN-1.3.8-vst2-aarch64.zip` and copy the folder **`Timo - VST - πClideaN`** into `VST2 Plugins` at the root of a drive, then restart MPC. On an MPC Live III the internal drive is `/nvme/VST2 Plugins`. Its `plugin.json` declares the 64-bit binary (`file64`).

## Gen2 skin note

MPC Live III draws a non-button `Image` component as a blank box when its bitmap is smaller than about 5400 pixels in area. The Gen2 package therefore ships a skin whose small images are padded to at least 6400 pixels. The Gen1 skin is unchanged.

Thomas Phillips tested the original 1.2.4 port on an MPC Live III. Rebuild: `scripts/build_aarch64.py` with `PICN_AARCH64_SYSROOT` (GCC 12 AArch64 sysroot) or Docker ARM64.

Please report device model, OS version and results.
