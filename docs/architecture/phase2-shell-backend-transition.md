# Sonarium Phase 2 Addendum — Prototype Shell Backend Transition (X11 → GLFW)

## Why the X11-specific shell was replaced
The original graphical prototype used a raw Xlib event/draw loop. That was acceptable for validating the earliest interaction concept, but it was tied to a native X11-only path and did not align with modern Linux desktop deployment where Wayland is common.

To keep momentum without redesigning DSP architecture, the prototype shell was migrated to GLFW:

- preserves a lightweight prototype footprint,
- keeps the UI minimal and interaction-first,
- removes direct dependency on X11-specific window/event APIs,
- enables viability across both Wayland and X11 sessions through GLFW backends.

## What did *not* change
This transition is strictly about the prototype shell layer.

Unchanged by design:

- DSP core (`stft`, `spectral_gain_mask_processor`),
- engine/state boundary and synchronization via `EngineFacade`,
- gain-mask interaction concept (click-set + drag-ramp edits),
- single-processor scope and non-plugin status.

## New prototype shell approach
The prototype now uses:

- GLFW for window creation, input, and context setup,
- a minimal OpenGL immediate-mode drawing path for line plots,
- the same edit -> engine facade update -> recompute loop.

This keeps implementation effort small while de-risking backend portability.

## Build/dependency notes
At configure/build time the prototype requires:

- `glfw3` (>= 3.3),
- OpenGL development libraries.

CMake target wiring:

- `find_package(glfw3 3.3 REQUIRED)`
- `find_package(OpenGL REQUIRED)`
- prototype target links `glfw` and `OpenGL::GL`.

## Wayland/X11 behavior and limitations
Expected runtime behavior on Linux:

1. In native Wayland sessions, GLFW generally uses its Wayland backend when available.
2. In X11 sessions, GLFW uses X11.
3. In Wayland sessions lacking required Wayland support, runtime may fall back via XWayland depending on environment and distribution packaging.

Prototype limitations in this pass:

- no custom Wayland protocol integration,
- no IME/text editing UI stack,
- no advanced high-DPI text rendering (plots remain functional; visuals are intentionally minimal),
- no persistent window state/session management.

These limitations are acceptable for the interaction-validation prototype and can be revisited in a later UI architecture pass.
