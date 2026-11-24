# πClideaN 1.4.1

- **Factory bank = INIT + the 150 Patternarium kits of πToniC**, in the same order and with the same names, so program N of πClideaN plays the pattern of kit N of πToniC (lines 1–8 use notes 36–43, the eight πToniC voices). Each program is INIT plus the kit's A+B pattern on lines 1–8 (steps, accents at 127, step rate, mutes); lines 9–16 stay as in INIT. A+B length is A plus B, as in MicroTonic: 127 kits at 32 steps, 20 at 24, 2 at 16, 1 at 10. INIT remains the default program. The six previous programs (Straight Four … Step Expression) are removed from the menu; saved projects keep their complete state.
- **MicroTonic pattern import**: new **IMPORT .MTPRESET** button on the CONFIG page. Pick a `.mtpreset` file, then one pattern A–L or two consecutive patterns (A+B, B+C … K+L) up to 32 steps; it replaces lines 1–8 like RDM 8, with the file's step rate and mutes. Notes, GATE and HOLD are kept; lines 9–16 are untouched. The pattern reader gives the same results as the M-VAVE FM-1 f-ToniC importer.
- Patterns derived from the Patternarium kits by Sonic Charge, made for MicroTonic.

UID, 4 096 published parameter indices, project format and MIDI routing are preserved. Gen2 remains untested on hardware.
