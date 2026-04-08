# Sonarium (Foundation + Spectral Interaction Prototype)

This repository contains Sonarium's early DSP core plus a minimal graphical spectral interaction prototype.

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

The prototype opens a lightweight X11 window with three live graphs:

- pre spectrum (dB),
- gain-mask curve (dB),
- post spectrum (dB).

Interaction:

- Left click in the mask plot: set a node.
- Left drag in the mask plot: draw a ramp across nodes.
- Press `r`: reset mask.
- Press `q`: quit.

Edits are routed through `EngineFacade` state APIs and immediately trigger post-spectrum recomputation.

See `docs/architecture/phase2-spectral-interaction-prototype-pass.md` for scope, boundaries, limitations, and next-step recommendations.
