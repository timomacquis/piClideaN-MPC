# Release procedure — both generations, standard packages only

Every public release must ship **both** official Open MPC packages on the same tag:

- `ClideaN-MPC-X.Y.Z-mpc-armv7.zip` — Gen1 / Force, ARM32.
- `ClideaN-MPC-X.Y.Z-mpc-aarch64.zip` — Gen2, ARM64.

Do not publish the former custom portable archives as an alternative installation path. The catalog entry matches the ARMv7 name and discovers the AArch64 sibling automatically.

## Required sequence

1. Set the release version and update the manuals and release notes. Preserve published parameter indices and the Gen2-only image workaround.
2. Run the relevant engine, MIDI, UI and state tests. Record device results separately from offline checks; never copy a prior device test or CPU result to a new package.
3. Build both architectures using `scripts/build.py` and `scripts/build_aarch64.py`.
4. Run `scripts/package.py` and `scripts/package_aarch64.py`. Both delegate to unmodified upstream `release.py`, pinned under `third_party/open-mpc-release`, and run `catalog_check.py --catalog --require-portable`.
5. Compare packaged engine and native skin bytes with the tested builds. Check each package's id, uid, version, source repo, architecture, checksums and official installer scripts.
6. Simulate install/reinstall/uninstall in isolated local settings, including architecture-specific registration. Such simulations are not device tests.
7. Make a complete local release backup: editable sources, both installation packages, checksums, manuals and validation records. Candidates use differential backups instead.
8. Publish the two standard packages, their SHA-256 files and complete sources to one GitHub release/tag. Verify GitHub asset digests. Keep the preceding release available under its original tag.
9. The catalog checks releases nightly. Passing local checks does not mean the public catalog has already refreshed. No new registry PR is needed for a version update that preserves identity and matching asset names.

For 1.3.1, the already-published engine and skin are repackaged byte-for-byte; only the distribution format and standard metadata/scripts change. Provenance records distinguish the runtime build commit from the later packaging-workflow commit.

Updating the packaging tool pin requires review of its source and checksums. Do not modify upstream installer templates locally to bypass a catalog failure.
