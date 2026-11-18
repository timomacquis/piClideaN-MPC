# Release procedure — Akai SDK plugin folders, both generations

Rule (17 November 2026, all plugins): a plugin is installed **only** as one folder in `VST2 Plugins`, in the format of Akai's plugin SDK (`plugin.json`, binary, `version.xml`, `Plugin Skins/`). No catalogue `install.sh`, no `Synths`, no MPC.settings.

Every public release ships on one tag:

- `piClideaN-X.Y.Z-mpc-sdk.zip` — one folder for Gen1 / Force (ARM32, `file32`) and Gen2 (ARM64, `file64`); also listed by the Open MPC catalog.
- `piClideaN-MPC-X.Y.Z-sources.zip`, and a `.sha256` file for each asset.

## Sequence

1. Set the version; update manuals and release notes. Never change the UID or published parameter indices.
2. Run the engine, MIDI, UI and state tests.
3. Build Gen1 (`scripts/build.py`) and Gen2 (`scripts/build_aarch64.py`), then assemble each SDK folder and ZIP.
4. Install the exact Gen1 folder on a Force and check it; record device results separately from PC checks.
5. Make a complete local backup, update the public repository, tag, publish the release with all assets and verify the downloaded SHA-256 values.
