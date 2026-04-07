# Sonarium Phase 2 — Minimal Spectral Interaction Prototype Pass

## Scope and intent
This pass delivers the smallest end-to-end interaction prototype proving Sonarium's core gesture:

- visualize spectral content,
- visualize a drawable gain-mask shape,
- edit mask values directly,
- route edits through engine state boundaries,
- hear/inspect meaningful before/after response.

This is intentionally **not** a final product UI, plugin wrapper, modulation pass, or multi-processor pass.

## What was implemented

### 1) Prototype visualization/editor harness
A new executable, `sonarium_spectral_interaction_prototype`, provides a lightweight terminal-based interaction loop for rapid validation:

- pre spectrum sparkline,
- post spectrum sparkline,
- delta sparkline,
- current mask (in dB) sparkline,
- numeric band summary for quick inspection.

The harness generates a mixed-tone demo signal and updates the display after every edit command.

### 2) Minimal direct gain-mask editing model
The prototype supports:

- `set <node> <value>` for direct point edits,
- `drag <start> <end> <start_value> <end_value>` for simple interpolated strokes,
- `reset` for restoring default mask,
- `show` for inspection without editing.

This interaction model is deliberately narrow and readable: enough to evaluate the gesture without introducing a full editor architecture.

### 3) Clean state bridge through `EngineFacade`
`EngineFacade` now owns a canonical `EngineState` snapshot and exposes minimal state-safe mutation APIs:

- `update_state(...)`
- `set_gain_mask_node(...)`
- `set_gain_mask_nodes(...)`
- `state()`

All edits synchronize DSP processor parameters through `sync_processors_from_state()`. The prototype app does not reach into DSP classes directly.

## Architectural notes

- DSP remains in `core` (`stft`, processors, state structs).
- The prototype interaction layer is in `src/app` and talks only to `engine::EngineFacade`.
- The visualization is intentionally temporary and text-based, but the state/control seam mirrors the intended UI→core command path.

## Validation workflow

```bash
cmake -S . -B build
cmake --build build
./build/sonarium_spectral_interaction_prototype
```

Try these quick checks:

1. `set 48 0.15` (attenuate high mask node) and observe high-band post/delta changes.
2. `drag 0 63 0.8 0.2` and observe broad spectral tilt.
3. `reset` to confirm state ↔ output consistency.

## Temporary limitations

1. Visualization is terminal-only (no final graphics backend yet).
2. Spectrum analysis in the prototype uses a lightweight correctness-first DFT helper, not a realtime UI pipeline.
3. No undo/redo stack.
4. Single processor only.
5. No dedicated downsampled analysis stream from STFT yet.

These are intentional for a disciplined prototype pass.

## Recommended next pass
After this prototype, prioritize a dedicated **analysis data bridge pass**:

1. add decimated pre/post analysis buffers emitted from core/engine for UI consumption,
2. keep command-driven mask editing API and begin translating to a lightweight 2D view,
3. add basic edit history (undo/redo semantic commands),
4. preserve current state boundary while preparing for standalone wrapper integration.
