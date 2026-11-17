# πClideaN 1.3.8 — maintenance

- **Installation follows Akai's SDK rule**: each release is a plugin folder to copy into `VST2 Plugins` (Gen1 ARM32 and Gen2 ARM64 ZIPs). No installer script, no `Synths` folder, no MPC.settings registration. Remove an older catalogue installation with its own `uninstall.sh` first.
- Code review fixes: the asynchronous waveform preview is enabled only once its worker thread really exists; the guidance worker initialises its scratch engine; dead parameter branches removed.
- Colour charter: blue outline on selectors without a neutral value (WAVE, SYNC, RATE, track MODE); amber once MOD DESTINATION leaves NONE or COUNT-IN leaves OFF; purple chained destinations unchanged.
- Thinner orange playhead outline on steps; larger MOD slider values; "Rdm 8" renamed "RDM 8"; track-page Q-Links limited to on-screen knobs.

UID, 4 096 published parameter indices, project format, factory programs and MIDI routing are preserved. Gen2 remains untested on hardware.
