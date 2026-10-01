# Frame Window Counter

A compact, cross-platform Geode mod for Geometry Dash 2.2081 that:

- captures player press/release events through `PlayerObject` hooks;
- timestamps them on a 240 Hz logical clock;
- displays animated markers and a live HUD;
- imports `.gdr`, `.gdr2`, `.slc`, `.json`, and `.fwc`;
- supports manual FW stamping;
- calculates an approximate `σ/s` precision for a 24-hour target;
- keeps the HUD under `OverlayManager` across scene changes;
- builds with Geode's official multi-platform GitHub Action.

## The key technical distinction

The input hook tells us **when** an input happened. It does not reveal the true acceptance interval of that timing. Frame-window size must therefore come from measured data, `.fwc`, NaNDL JSON, or manual assignment. GDR/SLC imports intentionally preserve the FW as unknown instead of assuming FW1.

## Build locally

```bash
geode build
```

## GitHub Actions

The workflow builds Windows, macOS, iOS, Android32 and Android64 independently, then combines the artifacts into one `.geode`.

## Termux upload

See `TERMUX.md`.
