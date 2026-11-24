# Gen2 ARM64 — πClideaN 1.4.1

The package also holds a binary compiled for 64-bit Gen2 MPC devices (`uname -m` prints `aarch64`). **Not tested on Gen2 hardware.**

Install exactly as on Gen1 (see [Installation and removal](INSTALLATION.md)): unzip `piClideaN-1.4.1-mpc-sdk.zip` and copy the folder **`Timo - VST - πClideaN`** into `VST2 Plugins` at the root of a drive, then restart MPC. On an MPC Live III the internal drive is `/nvme/VST2 Plugins`. Its `plugin.json` declares the 64-bit binary (`file64`).

## Gen2 skin note

MPC Live III draws a non-button `Image` component as a blank box when its bitmap is smaller than about 5400 pixels in area. The shared skin therefore pads small images to at least 6400 pixels; this changes nothing on Gen1.

Thomas Phillips tested the original 1.2.4 port on an MPC Live III. Rebuild: `scripts/build_aarch64.py` with `PICN_AARCH64_SYSROOT` (GCC 12 AArch64 sysroot) or Docker ARM64.

Please report device model, OS version and results.
