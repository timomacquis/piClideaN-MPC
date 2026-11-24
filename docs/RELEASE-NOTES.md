# πClideaN 1.4.1

- **Factory bank = INIT + the 150 Patternarium kits of πToniC**, in the same order and with the same names, so program N of πClideaN plays the pattern of kit N of πToniC (lines 1–8 use notes 36–43, the eight πToniC voices). Each program is INIT plus the kit's A+B pattern on lines 1–8 (steps, accents at 127, step rate, mutes); lines 9–16 stay as in INIT. A+B length is A plus B, as in MicroTonic: 127 kits at 32 steps, 20 at 24, 2 at 16, 1 at 10. INIT remains the default program. The six previous programs (Straight Four … Step Expression) are removed from the menu; saved projects keep their complete state.
- **MicroTonic pattern import**: new **IMPORT .MTPRESET** button on the CONFIG page. Pick a `.mtpreset` file, then one pattern A–L or two consecutive patterns (A+B, B+C … K+L) up to 32 steps; it replaces lines 1–8 like RDM 8, with the file's step rate and mutes. Notes, GATE and HOLD are kept; lines 9–16 are untouched. The pattern reader gives the same results as the M-VAVE FM-1 f-ToniC importer.
- Patterns derived from the Patternarium kits by Sonic Charge, made for MicroTonic.

UID, 4 096 published parameter indices, project format and MIDI routing are preserved. Gen2 remains untested on hardware.

---

# πClideaN 1.3.8 — maintenance

- **Installation follows Akai's SDK rule**: each release is a plugin folder to copy into `VST2 Plugins` (Gen1 ARM32 and Gen2 ARM64 ZIPs). No installer script, no `Synths` folder, no MPC.settings registration. Remove an older catalogue installation with its own `uninstall.sh` first.
- Code review fixes: the asynchronous waveform preview is enabled only once its worker thread really exists; the guidance worker initialises its scratch engine; dead parameter branches removed.
- Colour charter: blue outline on selectors without a neutral value (WAVE, SYNC, RATE, track MODE); amber once MOD DESTINATION leaves NONE or COUNT-IN leaves OFF; purple chained destinations unchanged.
- Thinner orange playhead outline on steps; larger MOD slider values; "Rdm 8" renamed "RDM 8"; track-page Q-Links limited to on-screen knobs.

UID, 4 096 published parameter indices, project format, factory programs and MIDI routing are preserved. Gen2 remains untested on hardware.

---

# πClideaN 1.3.7 — maintenance

- Bound preset and project recall notifications to eight host value callbacks per audio block, including momentary-button releases.
- Prevent retired eight-lane selector feedback from changing a restored A13 selection to A08.
- Capture a coherent musical state under the shared engine mutex, then serialize the project outside that mutex.
- Synchronize wrapper control transactions while keeping host notification callbacks outside the engine mutex.
- Let CHECK MIDI clear after a transient error has recovered; keep pending note-release warnings and the separate SHARED NOTE indication.
- Include the official architecture-specific plugin.json alongside standard catalog installers and legacy registration metadata.

Editor selection remains under touch control during MIDI playback.

UID, 4 096 published parameter indices, project formats, factory programs, skin and outgoing MIDI routing are preserved. Gen1 ARM32 and Gen2 ARM64 packages are provided. See VALIDATION-1.3.7.md for the scope of checks; Gen2 remains untested on hardware.

---

# πClideaN 1.3.6

- Replace full-page selectors with compact panels sized to their contents.
- Keep the current screen visible beneath a dim overlay, with the opening selector highlighted in yellow.
- Use 36 px buttons inside panels while preserving the main screen's control sizes.
- Centre all 32 step cells in the VARIATIONS panel.
- Apply the shared panel design to MOD selectors, SCALE, playback modes, variations and Rdm 8 confirmation.

Sequencing behaviour, the 4096 parameter indices, saved musical state, native presets and Q-Link assignments are preserved.

Gen1 ARMv7 and Gen2 AArch64 packages are provided. The Gen1 skin was installed on the author's Force; offline checks cover panel geometry, original control bindings, parameter identity and Q-Links. No new exhaustive touch/audio or exact-package install/uninstall qualification is claimed. Gen2 remains untested on hardware for this update.
