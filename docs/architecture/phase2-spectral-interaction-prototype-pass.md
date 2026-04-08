# Sonarium Phase 2 — Minimal Spectral Interaction Prototype Pass

## Scope and intent
This pass keeps the prototype intentionally small, but now clearly interactive:

- visualize spectral content,
- visualize an editable gain-mask shape,
- edit mask values directly with immediate visual feedback,
- route edits through the existing engine/state boundary,
- inspect meaningful before/after response.

This is still **not** a production UI layer, plugin wrapper, modulation pass, or multiprocessor UI.

## What was implemented

### 1) First real interactive experience in the graphical prototype
`sonarium_spectral_interaction_prototype` now presents obvious interaction affordances rather than a passive render.

The window displays:

- **pre spectrum** (dB),
- **gain mask** curve (dB),
- **post spectrum** (dB),
- compact on-screen help and live node/value readout.

### 2) Mouse interaction model
Inside the mask panel:

- **hover** highlights the panel,
- **left click** sets the nearest node value,
- **left drag** paints an interpolated ramp across traversed nodes,
- **right click** resets the hovered node,
- **shift + right click** resets the whole mask.

Keyboard helpers:

- `r` resets mask,
- `q` or `esc` quits.

### 3) Visible feedback loop
Every edit triggers:

1. state update through engine API,
2. mask redraw,
3. post-spectrum recomputation and redraw.

Additional visual feedback:

- filled mask area and 0 dB baseline,
- active/hover node indicators,
- cursor-local value tooltip.

### 4) Lightweight runtime diagnostics
The prototype now logs:

- GLFW init outcome,
- window creation success/failure,
- graceful shutdown,
- optional interaction event logs when `SONARIUM_PROTOTYPE_DEBUG=1` is set.

### 5) Clean engine/state boundary preserved
UI code still does not mutate DSP internals directly.

All mask edits are routed through:

- `EngineFacade::set_gain_mask_nodes(...)`
- `EngineFacade::update_state(...)` (reset path)

`EngineFacade` remains the state synchronization seam for processor parameters.

## Build and run

```bash
cmake -S . -B build
cmake --build build
./build/sonarium_spectral_interaction_prototype
```

Enable verbose interaction diagnostics:

```bash
SONARIUM_PROTOTYPE_DEBUG=1 ./build/sonarium_spectral_interaction_prototype
```

## Architectural notes

- DSP remains in `core` (`stft`, processors, state structs).
- Engine/state boundary remains in `engine::EngineFacade`.
- UI code lives in `src/app` and only talks to `EngineFacade`.
- Analysis remains a lightweight DFT helper suitable for prototype visualization.

## Temporary limitations

1. Immediate-mode drawing shell is prototype-only and not a production widget stack.
2. Text rendering is intentionally minimal for diagnostics/help only.
3. No undo/redo command history.
4. No dedicated decimated engine-owned analysis stream yet.
5. Single processor scope only.

## Recommended next step
Next pass should focus on a **UI-analysis bridge refinement + command semantics**:

1. emit reusable analysis frames from engine/core for UI consumption,
2. replace prototype plotting prep with reusable graph data adapters,
3. add semantic edit commands and undo/redo,
4. preserve the existing `EngineFacade` seam unchanged.
