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

Optional debug interaction logs:

```bash
SONARIUM_PROTOTYPE_DEBUG=1 ./build/sonarium_spectral_interaction_prototype
```

The prototype opens a lightweight GLFW window shell with three live graphs:

- pre spectrum (dB),
- gain-mask curve (dB),
- post spectrum (dB).

### Controls

- Move cursor over mask panel: hover highlight + nearest node/value readout.
- Left click in mask panel: set nearest node.
- Left drag in mask panel: draw interpolated ramp across nodes.
- Right click in mask panel: reset hovered node to default.
- Shift + right click in mask panel: reset entire mask.
- Press `r`: reset mask.
- Press `q` or `esc`: quit.

### What to verify quickly

1. Edit the mask with left drag and confirm the red mask curve updates immediately.
2. Observe the green post spectrum change as the mask shape changes.
3. Use right click resets to validate local/global reset behavior.

Edits are routed through `EngineFacade` state APIs and immediately trigger post-spectrum recomputation.

## Known limitations

- Prototype UI is immediate-mode and intentionally minimal.
- Text/help overlay is diagnostic-grade, not production typography.
- No undo/redo yet.
- No plugin wrapper integration.

Backend notes:

- On Linux sessions running Wayland, GLFW will use Wayland when available.
- On Linux sessions running X11 (or Wayland with XWayland fallback), GLFW will run on X11.

See `docs/architecture/phase2-spectral-interaction-prototype-pass.md` and
`docs/architecture/phase2-shell-backend-transition.md` for scope, boundaries, migration rationale, and known limitations.
