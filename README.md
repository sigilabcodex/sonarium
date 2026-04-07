# Sonarium (Foundation + Spectral Interaction Prototype)

This repository contains Sonarium's early DSP core plus a minimal spectral interaction prototype.

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Test

```bash
ctest --test-dir build --output-on-failure
```

## Run offline harness

```bash
./build/sonarium_offline_harness
```

The offline harness runs a generated tone through STFT + spectral gain mask processing and prints a peak-level sanity value.

## Run spectral interaction prototype

```bash
./build/sonarium_spectral_interaction_prototype
```

The prototype provides a terminal visualization/editor loop for the gain-mask interaction:

- pre/post/delta spectrum snapshots,
- current gain-mask curve view,
- direct `set` and `drag` commands that edit the mask through `EngineFacade` state APIs.

See `docs/architecture/phase2-spectral-interaction-prototype-pass.md` for scope, boundaries, and next steps.
