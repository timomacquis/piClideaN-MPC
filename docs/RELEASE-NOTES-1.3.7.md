# πClideaN 1.3.7 — maintenance

- Bound preset and project recall notifications to eight host value callbacks per audio block, including momentary-button releases.
- Prevent retired eight-lane selector feedback from changing a restored A13 selection to A08.
- Capture a coherent musical state under the shared engine mutex, then serialize the project outside that mutex.
- Synchronize wrapper control transactions while keeping host notification callbacks outside the engine mutex.
- Let CHECK MIDI clear after a transient error has recovered; keep pending note-release warnings and the separate SHARED NOTE indication.
- Include the official architecture-specific plugin.json alongside standard catalog installers and legacy registration metadata.

Editor selection remains under touch control during MIDI playback.

UID, 4 096 published parameter indices, project formats, factory programs, skin and outgoing MIDI routing are preserved. Gen1 ARM32 and Gen2 ARM64 packages are provided. See VALIDATION-1.3.7.md for the scope of checks; Gen2 remains untested on hardware.
