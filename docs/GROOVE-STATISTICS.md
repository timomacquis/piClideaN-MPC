# Groove-derived preferences — 1.3.4-candidate.2

πClideaN uses **457 bytes of constant integer tables** extracted offline from Groove MIDI Dataset v1.0.0. No raw MIDI/audio, neural model, TensorFlow dependency, network connection or user-side PC is required during playback. Search still runs only on an explicit ALTER/I RdM 8 request, on the existing background worker.

## What changes

ALTER adds a soft preference for observed cyclic spacing and, only for 16/32-step sixteenth-note cycles, observed metric positions. Similarity, selected-field protection, existing locks and the 75% effective attack ceiling remain authoritative. Cyclic spacing is normalized by cycle length; no 4/4 phase preference is imposed on a 7/10/13-step lane.

I RdM 8 draws density targets from the observed ranked-density distributions. Ranks describe presence, not instruments: A01–A08 are not assigned kick/snare/hat identities and MIDI notes/scales are untouched. Candidate evaluation adds pair-overlap preferences conditioned on actual event density across the existing four-bar timeline, retaining a saturation penalty. Shared attacks are allowed. Cached 128-bit event masks avoid repeatedly scanning previous lines for every candidate.

The interface, parameter table, presets, A/B scope, persistence and protection of 09–16 are unchanged from candidate.1. No velocity, swing, microtiming or harmonic learning is added by this step.

## Extraction and limits

- Official MIDI-only archive SHA-256: `651cbc524ffb891be1a3e46d89dc82a1cecb09a57c748c7b45b844c4841dcc1e`.
- Fit **train only**, beat performances in 4/4, complete nonempty bars. Mapped instrument families are merged, quantized to the nearest sixteenth on the absolute timeline; identity is discarded afterwards. Only lanes with 1–12 active positions per bar enter the statistics.
- Included training data: **368 performances, 16,356 bars and 66,901 active lane-bars**. These are filtered/quantized observations, not the full raw dataset.
- Equal weight per represented style, then per performance within style, then per bar; gaps are normalized per lane and pairs per bar. Missing density ranks use the sparsest active lane in that bar. Pseudocounts and a five-bin triangular smoothing of overlap counts prevent hard exclusions.
- Eight density ranks, three density bands, cyclic gaps, phase and overlap tables. Integers and inverse-CDF sampling replace runtime logarithms.
- Source and reproducible extraction: `scripts/extract_groove.py`, Python with `mido==1.3.3`; output recipe/counts in `reference/groove/EXTRACTION.json`.

Validation and test performances are kept out of fitting. Their metric-phase costs are lower than independently rotated controls, and cyclic-gap costs are lower than random positions matched for density. Pair-overlap results are mixed: one held-out split favors the real alignment, the other slightly favors rotated controls. These checks establish reproducibility and a statistical signal, **not musical superiority**. The corpus is predominantly drumming in 4/4; applying normalized spacing and anonymous density relations to polymetric or melodic use is a design adaptation, not a learned harmonic model.

Actual native gestures, listening preference, live switching and crackle-free operation remain to be checked on Force. ARM64 hardware qualification remains separate. The previous candidate is retained locally for comparison.

## Attribution

Derived tables retain **CC BY 4.0** attribution to Google LLC and the Groove MIDI Dataset contributors. See [the attribution and modifications](../licenses/GROOVE-ATTRIBUTION.md) and [the dataset](https://magenta.tensorflow.org/datasets/groove). Paper: Gillick, Roberts, Engel, Eck and Bamman, *Learning to Groove with Inverse Sequence Transformations*, ICML 2019. Original generation/extraction code remains MIT. No endorsement is implied.

The raw Groove MIDI Dataset archive is not distributed in this source repository or plugin package. Obtain the attributed upstream dataset separately to reproduce offline extraction. Runtime tables are already included in src/groove_tables.h.
