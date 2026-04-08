# Sonarium Phase 2 — Minimal Spectral Interaction Prototype Pass

## Scope and intent
This pass delivers the smallest end-to-end interaction prototype proving Sonarium's core gesture:

- visualize spectral content,
- visualize a drawable gain-mask shape,
- edit mask values directly,
- route edits through engine state boundaries,
- inspect meaningful before/after response.

This is intentionally **not** a final product UI, plugin wrapper, modulation pass, or multi-processor pass.

## What was implemented

### 1) First graphical prototype executable
`sonarium_spectral_interaction_prototype` is now a minimal native window prototype.

The window displays:

- **pre spectrum** (dB),
- **gain mask** curve (dB),
- **post spectrum** (dB).

The app uses an immediate redraw model: every edit triggers a state update and immediate re-render.

### 2) Mouse interaction model
Inside the mask panel:

- **click** sets a node value,
- **drag** paints an interpolated ramp between the last and current node,
- the mask curve redraws after each edit,
- post spectrum is recomputed and redrawn after each edit.

Keyboard helpers:

- `r` resets mask,
- `q` quits.

### 3) Clean engine/state boundary preserved
The window app does not mutate DSP internals directly.

All mask edits are routed through:

- `EngineFacade::set_gain_mask_nodes(...)`
- `EngineFacade::update_state(...)` (reset path)

`EngineFacade` remains the seam responsible for synchronizing processor parameters from state.

## UI approach chosen
The prototype uses a **minimal immediate-mode style native X11 loop**:

- single event loop,
- direct drawing of line plots,
- no heavyweight UI framework,
- small temporary code footprint suitable for prototyping.

This keeps the pass focused on validating interaction and state flow rather than framework architecture.

## Build and run

```bash
cmake -S . -B build
cmake --build build
./build/sonarium_spectral_interaction_prototype
```

## Architectural notes

- DSP remains in `core` (`stft`, processors, state structs).
- Engine/state boundary remains in `engine::EngineFacade`.
- UI code lives in `src/app` and only talks to `EngineFacade`.
- Analysis in this pass is still a lightweight DFT helper for legibility/correctness.

## Temporary limitations

1. Graphics path is a prototype-only native window, not a production UI layer.
2. No plugin wrapper integration.
3. No undo/redo command stack.
4. No dedicated decimated analysis stream from the STFT path.
5. Single processor scope only.

These limitations are intentional for this phase.

## Recommended next step
Next pass should focus on a **UI-analysis bridge refinement**:

1. emit reusable analysis frames from engine/core for UI consumption,
2. decouple plotting data prep from view rendering,
3. add semantic command history (undo/redo),
4. keep current engine boundary unchanged while preparing for future wrapper-facing UI.
