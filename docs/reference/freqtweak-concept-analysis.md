# FreqTweak Concept Analysis for Sonarium

## Purpose
This document captures **conceptual inheritance** from FreqTweak into Sonarium. It extracts product and interaction lessons, not implementation details.

## Source inspection summary
Primary reviewed sources:
- FreqTweak project README (GitHub mirror).
- Sonosaurus project page (feature descriptions, interaction model).
- Screenshot page and legacy UI screenshot.

Across sources, FreqTweak consistently presents itself as:
- FFT-based realtime spectral manipulation and display.
- A highly interactive filter-drawing GUI.
- A chain of unusual per-bin processors.
- A tool that is both “audio toy” and serious sound-design instrument.

## What FreqTweak fundamentally was
FreqTweak was a **realtime STFT spectral playground** that treated frequency bins as directly editable material. Instead of hiding processing behind abstract knobs, it let users:
- see spectral behavior (pre/post),
- draw processor shapes directly,
- attach modulators to spectral processors,
- iterate quickly between hearing and seeing.

In modern terms, it sat between:
- an effect plugin,
- a visual analysis tool,
- and a performance-oriented instrument.

## Why it was musically meaningful

### 1) Density of control
Per-bin editing at 64–4096 bands enabled textures and movement impossible with conventional EQ controls.

### 2) Immediate audiovisual coupling
Pre/post spectral views made non-intuitive DSP results legible. That shortened the “guess/listen/tweak” loop.

### 3) Expressive nonlinearity and artifacts
Processors like warp, per-bin delay/feedback, and pitch scaling intentionally enabled non-transparent outcomes.

### 4) Modulation as behavior, not decoration
Rotate/LFO/randomize modulators made processors dynamic and performable.

### 5) Range from subtle to extreme
It could do utility shaping, but invited exploratory and experimental practice.

## Ideas that still matter in 2026
- **Direct spectral drawing as primary interaction** (not secondary).
- **Pre/post visual trust model** for complex FFT processing.
- **Multi-resolution workflow** (band density, scale modes, zoom).
- **Bypass/link/A-B speed controls** to keep experimentation safe.
- **Modulation routability** across processors.
- **Preset-based exploration loop** for quick iteration and recall.

## Ideas to discard (or redesign heavily)
- Legacy dense utility UI and low discoverability interactions.
- Weak guardrails around unstable settings (e.g., spectral delay feedback).
- UX patterns requiring many hidden key-chord gestures to discover core behavior.
- Historical platform/toolkit assumptions as architectural constraints.
- Ambiguous processor semantics that hide when output is likely to become destructive.

## Processor concepts worth reinterpretation for Sonarium

### Strong carry-forward (v1 candidates)
1. **Draw EQ / gain sculpting**
   - Keep direct paint/draw interaction.
   - Add smoothing profiles and constrained tools (flat line, tilt, spline).

2. **Gate (double-threshold behavior)**
   - Preserve per-bin thresholding concept.
   - Modernize with clear threshold visualization, soft knees, and envelope timing controls.

3. **Per-bin spectral delay**
   - Preserve time-offset map concept.
   - Add explicit safety rails for feedback/energy growth.

4. **Warp (frequency remapping)**
   - Preserve “frequency→frequency reallocation” concept.
   - Add identity anchor mode, remap bounds, and anti-folding options.

5. **Pitch scaling map**
   - Preserve per-region pitch remap potential.
   - Offer musically constrained and free modes.

### Secondary carry-forward (post-v1)
- Per-bin limiter/compressor can return later as a carefully designed spectral dynamics suite, not an early v1 priority.

## Interaction model inheritance for Sonarium
Keep these principles:
- Draw-first editing on spectral surfaces.
- Fast modifier-enhanced precision actions.
- Explicit linked/unlinked channel behavior.
- Immediate comparison tools (bypass all, pre/post overlays, A/B).
- Resizable/zoomable views optimized for both macro gestures and micro correction.

Modernize with:
- Tool-state clarity, hints, and progressive disclosure.
- Fewer hidden interactions; more explicit visual affordances.
- Strong undo/redo and non-destructive workflow defaults.

## Sonarium conceptual inheritance decision
Sonarium should inherit FreqTweak’s **core thesis**:
> Spectral DSP is most musically useful when interaction is direct, visual, and realtime.

Sonarium should not inherit its historical constraints:
- retro utility ergonomics,
- fragile safety defaults,
- and monolithic architecture.

The right outcome is a **modern spectral instrument/processor** that is studio-credible yet creatively unconstrained.
