# Groove MIDI Dataset — attribution and derived statistics

Source: **Groove MIDI Dataset v1.0.0**, made available by Google LLC, with performances and contributions credited on the dataset page.

- Dataset: https://magenta.tensorflow.org/datasets/groove
- MIDI-only archive: https://storage.googleapis.com/magentadata/datasets/groove/groove-v1.0.0-midionly.zip
- Source SHA-256: `651cbc524ffb891be1a3e46d89dc82a1cecb09a57c748c7b45b844c4841dcc1e`
- License: **Creative Commons Attribution 4.0 International**, https://creativecommons.org/licenses/by/4.0/
- Paper: Jon Gillick, Adam Roberts, Jesse Engel, Douglas Eck and David Bamman, *Learning to Groove with Inverse Sequence Transformations*, ICML 2019.

πClideaN includes **modified, aggregated statistics**, not MIDI recordings or audio. The adaptation quantizes attacks to sixteenths, merges the documented drum families, discards instrument identity after extraction, filters compatible full 4/4 beat bars, balances styles/performances, and exports density distributions and integer cyclic-gap, phase and pair-overlap costs. Velocity and microtiming are not exported. Only the predefined training split fits these tables; validation and test remain held out.

The derived arrays in `src/groove_tables.h` and the extracted data in `reference/groove/EXTRACTION.json` retain **CC BY 4.0** attribution. Original extraction/generation code is covered by the project MIT license. No endorsement by Google, the authors or performers is implied.

Reproduction: use `scripts/extract_groove.py` with Python and `mido==1.3.3`, supplying the verified source archive. The raw dataset is not required at runtime and is not included in the installation ZIP.
