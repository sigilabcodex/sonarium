# Sonarium Architecture Proposal

## Goals
- Keep DSP, UI, and wrapper concerns cleanly separated.
- Enable standalone + plugin targets without duplicating core logic.
- Preserve realtime safety and predictable latency.
- Support iterative expansion of processors/modulators.

## 1) DSP Core Module

### 1.1 STFT engine
Responsibilities:
- input buffering/windowing
- overlap-add reconstruction
- configurable FFT size and overlap
- per-channel spectral frame management

Design notes:
- deterministic processing block API (host block -> internal frame scheduler)
- explicit latency reporting from FFT size/overlap/window pipeline
- denormal-safe and SIMD-friendly inner loops

### 1.2 Processor chain
Responsibilities:
- ordered list of spectral processors operating on frame buffers
- bypass, reorder policy (v1 may lock order; architecture should permit future reordering)
- channel-link modes (dual mono / linked stereo behaviors)

Initial processors:
- draw EQ
- gate
- smear
- delay
- warp
- pitch map

### 1.3 Modulation system
Responsibilities:
- control-rate source generation (LFO, envelope follower, macro values)
- mapping/routing to processor parameters
- depth/scaling/offset shaping

Design notes:
- modulation graph remains lightweight in v1 (source -> mapping -> parameter)
- smoothing to avoid zipper noise
- deterministic evaluation order each audio quantum

### 1.4 Parameter system
Responsibilities:
- canonical parameter IDs and ranges
- normalized <-> engineering unit conversions
- automation-safe update pathway
- state serialization hooks

Design notes:
- lock-free or wait-free handoff from UI/host thread to audio thread
- sample-accurate automation optional; block-accurate minimum for v1

## 2) UI Module

### 2.1 Rendering layer
Responsibilities:
- spectrogram + magnitude rendering
- pre/post visual channels
- zoom and freeze display logic
- GPU-accelerated drawing path where available

Design notes:
- renderer consumes decimated analysis streams, never blocks audio thread
- fallback CPU path for environments without robust GPU contexts

### 2.2 Interaction layer
Responsibilities:
- spectral draw gestures
- selection, transform, constrained edits
- modulation assignment gestures
- undo/redo command stack integration

Design notes:
- gesture model should be tool-based but minimal (draw, select, warp, inspect)
- visible interaction states and value readouts

### 2.3 Editor panels
- processor panels (compact + expanded)
- modulation panel
- macro panel
- preset browser and A/B controls

## 3) Wrapper Layer

### 3.1 Standalone app
- audio I/O integration for Linux-first workflows (JACK/PipeWire path)
- device and routing management
- session state load/save

### 3.2 LV2 plugin
- Linux ecosystem interoperability and distribution path

### 3.3 CLAP plugin
- modern plugin-host capabilities, flexible automation/modulation support

### 3.4 Optional VST3
- cross-platform compatibility target after core stability

Wrapper principle: wrappers adapt host/runtime APIs to core engine contracts; they do not own DSP logic.

## 4) State System

### 4.1 Preset format
- versioned, human-inspectable format (e.g., JSON/TOML-like schema)
- includes processor parameters, chain state, visualization prefs
- forward-compatible migration hooks

### 4.2 Modulation mapping state
- source definitions
- target parameter references by stable IDs
- depth/curve/range metadata

### 4.3 Macro controls
- macro definitions and named intents
- many-to-many mapping to parameters with scaling transforms

## 5) Performance and Realtime Concerns

### 5.1 Latency
- explicit computation and reporting from STFT settings
- UI display of “quality vs latency” profile

### 5.2 FFT overlap strategy
- support practical overlap presets (e.g., 2x/4x/8x)
- guardrails against CPU overload from extreme combinations

### 5.3 GPU rendering
- isolate GPU work to UI thread/process
- avoid synchronous GPU readbacks in realtime paths

### 5.4 Thread safety
- no locks in audio callback hot path
- double-buffered/shared snapshot state for UI meters/analysis
- bounded queues for cross-thread events

## 6) Recommended repository-level module split

- `core/dsp` — STFT, processors, modulation runtime
- `core/params` — parameter schema and automation translation
- `core/state` — preset and mapping serialization
- `ui/render` — spectrogram/magnitude rendering
- `ui/editor` — interaction/tools/panels
- `wrappers/standalone`
- `wrappers/lv2`
- `wrappers/clap`
- `wrappers/vst3` (optional)

This split keeps Sonarium evolvable while preserving strict realtime and portability boundaries.
