# Sonarium First Technical Foundation Pass

## Scope and intent
This pass implements the smallest architecturally serious core aligned with the existing vision and architecture documents:

- STFT-domain processing path (analysis -> spectral processing -> inverse synthesis).
- Exactly one processor (`SpectralGainMaskProcessor`) with draw-mask-compatible control shape.
- Explicit parameter/state boundary and an engine facade suitable for future wrappers.
- Baseline DSP tests for reconstruction and stability.

This is intentionally **not** a UI pass, plugin pass, or full product feature pass.

## Implemented structure

- `src/core/stft`
  - `StftEngine` with Hann windowing, overlap-add reconstruction, configurable FFT/hop.
  - Current implementation uses a straightforward DFT/IDFT backend to keep dependency surface minimal in this foundation stage.
- `src/core/processors`
  - `SpectralGainMaskProcessor` applying per-bin gains interpolated from user-facing mask nodes.
- `src/core/params`
  - Normalized parameter helpers and mapping to engineering units (dB -> linear gain).
- `src/core/state`
  - `EngineState` and processor state payloads as the user-facing control layer.
- `src/engine`
  - `EngineFacade` as wrapper/UI-facing entry point, separating control updates from DSP internals.
- `src/app`
  - `offline_harness` for basic manual validation.
- `tests`
  - STFT roundtrip and processor behavior/stability checks.

## Why this first processor
The spectral gain mask was chosen first because it directly supports Sonarium's draw-first interaction thesis while remaining technically constrained:

- It exercises the full STFT analysis/process/synthesis loop.
- It establishes the parameter-to-spectral-bin mapping pattern needed for future drawable processors.
- It introduces minimal risk compared with more nonlinear processors (delay/warp/pitch-map).

## Current limitations / deliberate compromises

1. FFT backend is currently naive DFT/IDFT (correctness-focused, not realtime-optimized).
2. Current processing helper is mono for simplicity in this first pass.
3. No lock-free snapshot swap yet; state updates are explicit API calls and should become block-boundary snapshots in the next pass.
4. No modulation sources yet.
5. No UI bridge or plugin wrappers yet.

These are intentional to keep the foundation small and testable while preserving architecture boundaries.

## Build and test

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Optional harness run:

```bash
./build/sonarium_offline_harness
```

## Recommended next technical step
Execute a Phase 1.5 pass focused on realtime viability without expanding product scope:

1. Replace DFT with FFT backend abstraction (e.g., FFTW/KissFFT adapter layer).
2. Add per-channel processing path (linked stereo baseline).
3. Introduce immutable state snapshot swap at block boundaries.
4. Add no-allocation checks in process path and microbenchmarks for FFT profile budgets.
