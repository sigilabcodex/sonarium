# Sonarium Foundation Plan

## Planning intent
This roadmap emphasizes a strong foundation and testable milestones before full product breadth.

## Phase 0 — Architecture lock

### Deliverables
- architecture decision record (module boundaries, realtime constraints, plugin targets)
- parameter ID strategy and state schema draft
- processor order and v1 capability matrix

### Testable result
- architecture review sign-off with no unresolved critical boundary questions
- agreed latency and threading model documented

### Not yet in scope
- full UI implementation
- full plugin compatibility matrix validation

---

## Phase 1 — STFT DSP core prototype

### Deliverables
- functional STFT forward/inverse pipeline with overlap-add
- basic processor chain scaffold with bypass routing
- initial placeholder implementations for v1 processor interfaces

### Testable result
- offline and realtime smoke tests proving stable passthrough and bounded CPU use
- latency reporting from selected FFT/overlap settings

### Not yet in scope
- finished processor UX semantics
- advanced modulation routing

---

## Phase 2 — Spectrogram rendering prototype

### Deliverables
- pre/post spectral data feed from core to UI layer
- spectrogram and magnitude prototype widgets
- zoom/freeze primitives

### Testable result
- realtime display tracks input/output changes with acceptable frame rate and no audio thread stalls

### Not yet in scope
- polished visual design system
- full editor panel layout

---

## Phase 3 — Interactive spectral drawing

### Deliverables
- draw/select gesture tools on spectral controls
- smoothing/interpolation for drawn curves
- undo/redo command backbone for edit operations

### Testable result
- users can draw spectral EQ and hear predictable, immediate results with undo safety

### Not yet in scope
- full multi-tool workflow depth
- advanced warp map editing UX

---

## Phase 4 — Standalone app shell

### Deliverables
- standalone host shell with audio I/O setup and routing
- session open/save for local state
- baseline preset browser integration

### Testable result
- app launches, processes live audio, saves/restores sessions reliably

### Not yet in scope
- advanced device abstraction layers across all platforms
- extensive onboarding/tutorial system

---

## Phase 5 — Plugin wrapper

### Deliverables
- CLAP + LV2 wrapper integration around shared core
- host automation plumbing for exposed parameters
- basic validation in representative Linux hosts

### Testable result
- plugin loads, processes audio, and responds to automation in supported hosts

### Not yet in scope
- optional VST3 distribution readiness
- exhaustive host-by-host edge-case certification

---

## Phase 6 — Presets and modulation

### Deliverables
- finalized preset format v1 with versioning
- LFO and envelope follower modulation sources
- macro mapping and serialization

### Testable result
- mappings persist across save/load; modulation behaves deterministically under automation

### Not yet in scope
- large curated factory content library
- deep performance scripting system

---

## Phase 7 — UI refinement

### Deliverables
- visual polish pass (legibility, contrast, spacing, interaction clarity)
- improved inspector panels and workflow shortcuts
- accessibility and usability review fixes

### Testable result
- usability checklist completion with reduced error rate in common tasks

### Not yet in scope
- major scope expansion beyond v1 processor/modulation set
- full redesign of interaction model

---

## Cross-phase acceptance criteria
- Realtime safety is never regressed for feature gains.
- DSP and UI remain separable modules.
- Each phase ends with demonstrable, testable behavior.
- Deferred items remain documented to prevent silent scope creep.
