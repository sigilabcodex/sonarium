# Sonarium (Foundation Pass 1)

This repository now contains a first technical foundation for Sonarium's DSP core.

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

The harness runs a generated tone through the STFT + spectral gain mask processor and prints a simple peak-level sanity value.
