# Sonarium (Foundation + Spectral Interaction Prototype)

This repository contains Sonarium's early DSP core plus a minimal graphical spectral interaction prototype.

## Build

Dependencies for the graphical prototype:

- CMake 3.20+
- OpenGL development headers/libraries
- GLFW 3.3+ development package (Wayland/X11 backend support is provided by GLFW and distro build options)

Build:

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

The prototype opens a lightweight GLFW window shell with three live graphs:

- pre spectrum (dB),
- gain-mask curve (dB),
- post spectrum (dB).

Interaction:

- Left click in the mask plot: set a node.
- Left drag in the mask plot: draw a ramp across nodes.
- Press `r`: reset mask.
- Press `q`: quit.

Edits are routed through `EngineFacade` state APIs and immediately trigger post-spectrum recomputation.

Backend notes:

- On Linux sessions running Wayland, GLFW will use Wayland when available.
- On Linux sessions running X11 (or Wayland with XWayland fallback), GLFW will run on X11.
- Keyboard/mouse interaction is intentionally prototype-grade (single-window, immediate redraw, no text widget stack).

See `docs/architecture/phase2-spectral-interaction-prototype-pass.md` and
`docs/architecture/phase2-shell-backend-transition.md` for scope, boundaries, migration rationale, and known limitations.
