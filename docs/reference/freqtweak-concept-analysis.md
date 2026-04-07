# FreqTweak Concept Analysis (for Sonarium)

## Purpose of this document
This document captures **conceptual inheritance** from FreqTweak for Sonarium. It intentionally extracts ideas, interaction patterns, and DSP framing—not code, APIs, or UI style.

## What FreqTweak fundamentally was
FreqTweak was an FFT/STFT-based realtime spectral manipulation environment built around direct, visual control of per-bin behavior. In practice it combined:

- realtime spectral analysis (including pre/post views)
- gesture-based editing of spectral transfer curves
- multiple unusual per-bin processors (not just EQ)
- modulation of spectral processors over time

It behaved less like a traditional “plugin utility” and more like an exploratory performance/sound-design instrument.

## What made it musically interesting

1. **Per-bin manipulation at playable resolution**  
   It allowed users to shape many bins directly, creating textures and motion unavailable in low-bandwidth controls.

2. **Visual feedback tightly coupled to audio consequences**  
   Spectrogram and spectrum views made cause/effect immediate, supporting discovery.

3. **Processor designs that embraced strangeness**  
   Gate, delay, warp, and per-bin pitch operations could produce expressive artifacts, not just transparent correction.

4. **Modulation as a first-class operation**  
   LFO/rotation/randomization concepts encouraged motion and performance behavior rather than static settings.

5. **“Toy + serious tool” dual identity**  
   It welcomed experimentation while still being useful for real sound design and utility analysis.

## Valuable ideas to retain

- **Visual-first spectral editing workflow** where interaction happens on top of the spectrum, not hidden in parameter lists.
- **Pre/post comparative views** to keep processing trustworthy.
- **High-band-count editing** with zoom and scale options to balance precision and speed.
- **Processor chain of orthogonal spectral transforms** (gain/threshold/redistribution/time/pitch-domain).
- **Modulation routing** from generic modulators to multiple targets.
- **Preset-centric exploration loop** (save/load states quickly while auditioning).
- **Realtime low-latency mindset** suitable for musical interaction, not only offline rendering.

## What Sonarium should explicitly NOT inherit

- **Legacy desktop UI style and dense utility-era layout** (small controls, cramped rows, retro visual language).
- **Unbounded “unsafe by default” gain/feedback behavior** without modern guardrails and metering.
- **Weak separation of DSP state and interaction concerns** (Sonarium needs strict modular architecture).
- **Old platform assumptions** (single historical Linux stack only). Sonarium should be Linux-first but wrapper-ready across modern plugin ecosystems.
- **One-size processing semantics** that can cause accidental destructive settings without clear feedback.

## Spectral processors worth reinterpreting

### 1) Draw EQ (cut/boost)
Reinterpret as high-resolution draw/gesture EQ with smoothing, quantized gesture options, and optional spectral-domain interpolation.

### 2) Spectral gate
Reinterpret as per-bin dual-threshold regioning plus musically readable controls (range, softness, hold/release shaping).

### 3) Spectral delay
Reinterpret with per-bin time offset maps, stable feedback handling, and visual delay energy traces.

### 4) Spectral warp
Reinterpret as frequency-to-frequency remap matrix/curve with identity lock, local pinning, and anti-folding options.

### 5) Spectral pitch map
Reinterpret per-bin scaling as musically constrained pitch mapping with optional tempered anchors and freeform mode.

### 6) Spectral smear (new emphasis)
Not a direct FreqTweak primitive, but strongly aligned with its spirit: energy diffusion over time/frequency neighborhoods for textural shaping.

## Interaction ideas worth keeping

- **Direct drawing and dragging on spectral controls** as the primary gesture vocabulary.
- **Modifier-based precision gestures** (line draw, constrained move, alternate layer editing), but with discoverable modern affordances.
- **Bypass and link concepts** across channels/lanes for rapid A/B and stereo coherence.
- **Zoomable views with explicit scale modes** (linear/log/hybrid).
- **Fast “experimental loop” UX**: tweak → hear → see → undo/compare → store.

## Conceptual inheritance summary
Sonarium should inherit FreqTweak’s **instrumental attitude toward spectral DSP**: realtime, visual, and expressive. It should not inherit implementation-era limitations, retro control density, or unstable behavior defaults. The objective is a modern spectral instrument for professional workflows, not a historical emulation.
