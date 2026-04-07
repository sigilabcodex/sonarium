# Sonarium Architecture Proposal

## Architectural goals
1. Keep audio realtime paths deterministic and lock-free.
2. Decouple DSP core from UI and host wrappers.
3. Support standalone and plugin deployment from one shared core.
4. Make processor/modulation growth additive rather than structural rewrite.
5. Preserve clear boundaries for testing and performance profiling.

## 1) System decomposition

### 1.1 Core DSP domain (`core`)
Owns all audio processing and canonical state interpretation:
- STFT analysis/synthesis engine.
- Spectral processor chain runtime.
- Modulation runtime.
- Parameter transformation (normalized ↔ engineering units).
- Safety policies (clamps, saturation strategy, bounds).

### 1.2 Presentation domain (`ui`)
Owns visualization and editing semantics:
- Spectral rendering models.
- Interaction tools and edit gestures.
- Editor/panel composition.
- Non-audio command generation (edits, mappings, preset actions).

### 1.3 Host integration domain (`wrappers`)
Owns platform/format adaptation:
- Standalone audio/MIDI/device lifecycle.
- Plugin format ABI integration.
- Host automation transport and transport/tempo bridge.
- State handoff to/from core.

**Boundary rule**: wrappers and UI can request changes; only core executes DSP behavior.

## 2) STFT engine responsibilities and contract

### Responsibilities
- Windowing, overlap scheduling, FFT/iFFT execution, overlap-add reconstruction.
- Multi-channel frame handling.
- Latency accounting and reporting.
- Quality profile handling (FFT size, overlap, window choice).

### Contract
- Input: interleaved or planar audio blocks from host wrapper.
- Output: processed blocks with known algorithmic latency.
- Side outputs: decimated analysis buffers for UI consumption.

### Justification
STFT is the architectural fulcrum. Isolating it enables:
- precise performance profiling,
- deterministic latency behavior,
- future processor evolution without touching wrapper logic.

## 3) Processor chain model

### v1 model
- Fixed-order chain (predictable, easier QA).
- Per-processor bypass and wet/dry mix.
- Channel mode support: linked stereo vs dual-mono behavior where applicable.

### Runtime behavior
- Processors operate on spectral frames/buffers in deterministic order.
- Parameter snapshots are atomically swapped at block boundaries.
- Optional per-processor smoothing inside process blocks.

### Why fixed-order first
A reorderable graph is attractive but costly for test matrix and UX complexity. Fixed order provides musical consistency and faster stabilization for v1.

## 4) Modulation system boundaries

### In scope for v1
- Sources: LFO, envelope follower, macros.
- Mapping layer: source → transform(depth/offset/curve) → target parameter.
- Deterministic evaluation each audio quantum.

### Out of scope for v1
- Arbitrary modulation graph feedback networks.
- Per-sample modulation matrix editing UI complexity.

### Realtime model
- Source generation and mapping evaluation occur in audio-safe code.
- UI only edits mapping definitions through lock-free command/state channels.

## 5) Parameter and state architecture

### Parameter model
- Stable parameter IDs.
- Normalized [0..1] transport representation.
- Strong type metadata: float/int/enum/toggle.
- Conversion functions and display formatting in shared schema.

### State model
- Versioned preset/session payload with migration hooks.
- Serializable objects:
  - processor settings,
  - modulation mappings,
  - macro definitions,
  - UI view preferences (non-audio-critical).

### Synchronization strategy
- Audio thread reads immutable snapshots.
- UI/host writes to staging state.
- Atomic pointer/index swap at safe boundary.

## 6) UI and rendering boundaries

### Rendering pipeline
- UI consumes downsampled/decimated spectral data only.
- Renderer thread never blocks audio callback.
- GPU acceleration preferred; CPU fallback maintained.

### Interaction pipeline
- Gestures compile to semantic edit commands (not direct DSP mutation).
- Command stream updates core parameter/state model via thread-safe bridge.
- Undo/redo stores semantic edits, not raw buffers.

### Why this split
It allows rich interaction without contaminating realtime DSP with UI concerns.

## 7) Standalone vs plugin strategy

### Shared engine
- One `EngineFacade` style API consumed by all wrappers.

### Standalone wrapper (early)
- Linux-first audio I/O path (JACK/PipeWire).
- Device/routing/session lifecycle management.

### Plugin wrappers
- CLAP + LV2 first for Linux ecosystem and modern host support.
- VST3 as later compatibility expansion.

### Justification
Linux-first delivery aligns with project intent, while shared facade keeps cross-format cost bounded.

## 8) Likely implementation stack (recommended)
- Language: modern C++ (performance, ecosystem compatibility).
- FFT backend: FFTW or equivalent high-performance backend abstraction.
- Plugin layer candidates: CLAP/LV2 SDKs directly or JUCE-i/o-independent bridging where appropriate.
- UI rendering: GPU-capable immediate/declarative layer with explicit fallback path.
- Serialization: human-readable structured format (JSON/TOML) with schema version field.

(Exact framework selection should be locked in Phase 0 ADRs.)

## 9) Performance and realtime-safety risks

### High-risk areas
1. FFT size/overlap combinations causing CPU spikes.
2. Spectral delay/feedback maps causing runaway energy.
3. UI data transport backpressure causing callback jitter if poorly isolated.
4. Overly granular automation/modulation updates causing zipper artifacts.
5. Memory churn from dynamic allocations in audio path.

### Required safeguards
- No locks, no heap allocation, no blocking calls in audio callback.
- Hard parameter clamps and safety limits for unstable processors.
- Bounded queues/ring buffers for cross-thread data.
- Profiling gates and regression thresholds for CPU and xruns.

## 10) Testing strategy

### Unit tests
- STFT reconstruction error bounds.
- Parameter conversion and migration correctness.
- Modulation mapping math determinism.

### Property/invariant tests
- Processor output remains finite (no NaN/Inf) under randomized legal params.
- State roundtrip (serialize/deserialize) invariants.

### Integration tests
- Offline render snapshots for processor chains.
- Wrapper-core state synchronization behavior.
- Automation playback determinism for fixed seeds.

### Realtime/system checks
- Long-run xrun/callback-overrun monitoring at representative buffer sizes.
- CPU headroom benchmarks across quality profiles.
- UI stress with active spectral rendering while processing.

## 11) Recommended initial module layout
- `core/stft`
- `core/processors`
- `core/modulation`
- `core/params`
- `core/state`
- `engine` (facade + scheduling glue)
- `ui/render`
- `ui/editor`
- `wrappers/standalone`
- `wrappers/clap`
- `wrappers/lv2`
- `wrappers/vst3` (deferred)

This layout provides high cohesion inside domains and low coupling across domains, which is essential for Sonarium’s v1 scope discipline.
