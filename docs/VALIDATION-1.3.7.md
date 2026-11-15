# Validation — 1.3.7

Offline checks cover the actual engine and wrapper with simulated ALSA output under AddressSanitizer and UndefinedBehaviorSanitizer: ignored MIDI input, preset/project recall, bounded notifications including explicit zero releases, restored selection, immutable-state serialization, concurrent UI access, MOD, GATE, random density, scales, polymetric cycles and count-in.

Incoming musical events leave editor selection unchanged. No new exact-release device install/uninstall cycle or CPU qualification is claimed. Gen2 has no new hardware validation.

Both ELF architectures, fixed parameter indices, package identity, official plugin.json, runtime payloads and archive checksums are checked. Standard install/reinstall/uninstall scripts are simulated against disposable local settings, separately for ARM32 and ARM64. These simulations are not device tests.
