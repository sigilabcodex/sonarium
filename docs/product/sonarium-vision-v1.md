# Sonarium Product Vision v1

## Product statement
Sonarium is a **visual spectral sculpting instrument and processor** for realtime audio work. It is designed to feel like:
- a laboratory instrument (precision, readable state),
- a performance surface (playable modulation and gestures),
- and a sound-sculpting environment (creative spectral transformations).

It targets both:
- **standalone** use for direct experimentation/performance,
- **plugin** use for DAW-based production workflows.

Linux-first is a priority, while architecture remains wrapper-ready for broader host ecosystems.

## Who Sonarium is for

### Primary users
- Sound designers and electronic music producers.
- Mix engineers needing selective spectral shaping beyond traditional EQ.
- Live performers exploring expressive, visual effects.
- Experimental artists and electroacoustic practitioners.

### Secondary users
- Researchers/educators demonstrating spectral processing concepts.
- Audio developers evaluating novel spectral workflows.

## Core use cases
1. **Spectral sculpting**: remove, carve, accent, and contour frequency energy with direct drawing.
2. **Textural transformation**: create motion and texture using warp/smear/delay/pitch-map style processors.
3. **Dynamic spectral control**: drive processor behavior with LFOs, envelopes, and macro controls.
4. **Mix integration**: deploy as a plugin for targeted track/bus spectral work.
5. **Performance flow**: use macros and visual feedback for realtime expressive control.

## Product balance: instrument vs effect vs workstation

### Instrument
- Gesture-led editing and macro performance controls are central.
- Modulation is expressive and routable, not hidden.

### Effect processor
- Deterministic signal flow, automation-ready parameters, and reliable latency reporting.
- Plugin-compatible control/state model.

### Workstation-like environment (limited v1)
- Focused internal workflow for inspect → edit → compare → store.
- Not a full node graph or general audio workstation.

## v1 must include (non-negotiable scope)

### A) Spectral engine + display foundation
- STFT-based realtime processing with stable overlap-add reconstruction.
- Pre and post spectral visualization.
- Frequency scale modes and zoom.
- Freeze/inspect workflow.

### B) v1 processor set
- Draw EQ (cut/boost spectral shaping).
- Spectral Gate.
- Spectral Smear.
- Spectral Delay (with safe feedback handling).
- Spectral Warp.
- Spectral Pitch Map.

### C) Modulation and performance control
- LFO source.
- Envelope follower source.
- Macro controls (assignable to multiple targets).
- Deterministic modulation routing and smoothing.

### D) Workflow essentials
- Presets (save/load/versioned state).
- Undo/redo.
- A/B compare.
- Parameter exposure suitable for host automation.

### E) Product targets
- Linux-first professional audio workflow viability.
- Standalone shell + at least one modern plugin format foundation in v1 trajectory.
- Realtime-stable behavior and clear safety guardrails.

## Explicitly deferred beyond v1
- Full modular node-graph architecture.
- ML-assisted source separation/repair features.
- Deep MPE expression framework.
- Collaborative cloud preset ecosystem.
- Immersive multichannel/surround-first feature set.
- Large factory content ecosystem.
- Advanced spectral dynamics suite (multiband mastering-grade variants).

## v1 quality bar
Sonarium v1 succeeds only if it is:
- **Musically expressive** (not just analytical),
- **Operationally reliable** in realtime,
- **Visually legible** under complex processing,
- **Architecturally extendable** without major rewrites,
- **Aesthetically refined** (modern, intentional, non-retro-clunky).

## v1 anti-goals
- Not a generic spectrogram utility.
- Not a nostalgic clone of historical tools.
- Not a giant “do everything” spectral DAW.
- Not a plugin-format land grab before core interaction quality is proven.
