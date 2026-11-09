# Building for Gen2

Based on Thomas Phillips’s contribution in PR #2 (upstream commits ec24e397 through 5b9ee64). Engine, wrapper and parameter indices are unchanged.

Use Python 3 with Pillow and either Docker with ARM64 emulation or an extracted Debian Bookworm GCC 12 AArch64 cross-toolchain.

```sh
python3 scripts/build_aarch64.py
python3 scripts/package_aarch64.py
```

For a local cross-toolchain, set `PICN_AARCH64_SYSROOT` to its extraction root. This needs GCC 12, matching binutils and host libraries, arm64-cross libc/development packages, and the target `usr/lib/aarch64-linux-gnu/libasound.so.2`.

Gen2 outputs live in `build/aarch64/`; its skin and receipt are separate from the Gen1 outputs. The packaging script verifies the current source hashes, engine checksum and generated skin. It does not rewrite the Gen1 ZIP or root manifests.

`PICN_SKIN_MIN_IMAGE_AREA=6400` is enabled only by the Gen2 builder to work around the small Image-component issue observed on Live III. The default skin generation has no padding.

The 1.3.1 Gen2 package requires device testing; the original contribution’s hardware test applies to its 1.2.4 build.
