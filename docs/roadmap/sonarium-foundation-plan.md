# Sonarium Foundation Plan

## Planning principles
- Sequence for risk retirement, not feature vanity.
- End every phase with objective testable outcomes.
- Protect realtime safety and architectural boundaries at all phases.
- Defer breadth until core interaction and engine behavior are stable.

---

## Phase 0 — Architecture lock

### Deliverables
- Architecture Decision Records (ADRs) for:
  - module boundaries,
  - threading model,
  - state/parameter schema,
  - wrapper strategy (standalone, CLAP, LV2; VST3 deferred).
- v1 processor spec sheets (inputs, outputs, guardrails, expected ranges).
- performance budget draft (CPU/latency targets per quality profile).

### Testable
- Design review sign-off with no unresolved critical boundary decisions.
- Parameter ID registry frozen for v1.
- Prototype latency calculations validated for planned STFT profiles.

### Not yet attempted
- Full processor implementations.
- UI design polish.
- Multi-host plugin certification.

---

## Phase 1 — DSP kernel prototype

### Deliverables
- Functional STFT analysis/synthesis pipeline.
- Processor-chain runtime skeleton with bypass/wet-dry semantics.
- Minimal implementations (or stubs with deterministic behavior) for v1 processor interfaces.
- Realtime-safe parameter snapshot handoff mechanism.

### Testable
- Null/passthrough reconstruction tests with defined error tolerance.
- CPU profiling baseline for representative sample rates/buffer sizes.
- No-allocation/no-lock assertions in audio callback path.

### Not yet attempted
- Final musical tuning of processors.
- Complex modulation UI.

---

## Phase 2 — Spectral visualization prototype

### Deliverables
- Core→UI spectral data bridge (decimated analysis stream).
- Pre/post spectrogram + magnitude views.
- Zoom/freeze/scale-mode interaction primitives.

### Testable
- Visual display tracks audible changes with acceptable frame cadence.
- Audio callback remains stable under heavy UI redraw.
- Pre/post comparison visually coherent for known test signals.

### Not yet attempted
- Full styling system.
- Advanced edit tools.

---

## Phase 3 — Interactive spectral editing prototype

### Deliverables
- Draw/select tools for spectral gain-shaping.
- Constraint tools (line/tilt/snap) and smoothing behavior.
- Undo/redo command stack for edit operations.
- Initial channel-link handling.

### Testable
- A user can execute edit → audition → undo/redo loops reliably.
- Gesture latency and edit determinism meet interaction thresholds.
- No desynchronization between visual state and audible state.

### Not yet attempted
- Full warp-map authoring UX depth.
- Complete processor editor suite.

---

## Phase 4 — Standalone shell

### Deliverables
- Linux-first standalone host shell (audio device, routing, transport basics).
- Session save/load around shared engine state.
- Baseline preset browser and A/B compare path.

### Testable
- Cold start to audio pass-through within acceptable startup time.
- Stable live processing across common Linux audio setups.
- Session and preset roundtrip integrity.

### Not yet attempted
- Cross-platform installer/distribution polish.
- Advanced onboarding/tutorial systems.

---

## Phase 5 — Plugin wrapper foundation

### Deliverables
- CLAP and LV2 wrapper integration with shared engine facade.
- Host automation parameter mapping.
- Initial latency/reporting compliance behavior.

### Testable
- Plugin load/instantiate/process/unload stability in target hosts.
- Automation playback reproducibility for smoke scenarios.
- State save/restore in host projects.

### Not yet attempted
- Full VST3 distribution pipeline.
- Exhaustive host-by-host certification matrix.

---

## Phase 6 — Presets, modulation, macro control

### Deliverables
- Versioned preset format v1 (migration-ready).
- LFO + envelope follower modulation sources finalized.
- Macro mapping editor logic and serialization.
- Safety clamping/normalization across modulation targets.

### Testable
- Preset compatibility across minor schema revisions.
- Deterministic modulation behavior under automation playback.
- Macro movements produce bounded, artifact-controlled parameter changes.

### Not yet attempted
- Massive factory content program.
- Advanced scripting/modulation graph ecosystems.

---

## Phase 7 — UI refinement and workflow polish

### Deliverables
- Visual refinement pass (contrast, hierarchy, spacing, readability).
- Efficiency improvements for high-frequency tasks.
- Accessibility and ergonomics adjustments.
- Finalized v1 workflow defaults and safety feedback messaging.

### Testable
- Structured usability sessions for top workflows.
- Reduced user error and faster time-to-target vs earlier phase baseline.
- No regression in audio-thread stability from UI polish changes.

### Not yet attempted
- Major product scope expansion.
- Fundamental interaction model rewrites.

---

## Cross-phase exit criteria (always active)
- Realtime safety regressions block phase completion.
- Architectural boundaries remain enforced (no wrapper/UI DSP leakage).
- Each phase ships with reproducible checks and demo scenarios.
- Deferred scope remains documented to prevent silent scope creep.

## Best immediate next step after this documentation pass
Run a **Phase 0 architecture lock workshop** and produce ADRs for:
1. STFT profile set and latency policy,
2. parameter/state schema and IDs,
3. wrapper sequence (standalone, CLAP, LV2),
4. UI-to-core command/state bridge.

This single step unlocks implementation with minimal rework risk.
