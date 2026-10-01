# Frame Window Counter

A Geode utility for logging player inputs, visualizing known frame windows, importing common replay formats, and estimating an approximate **σ/s precision** needed for a 24-hour completion target.

## Important limitation

A `.gdr/.gdr2/.slc` file stores input timing, but not the measured frame-window tolerance itself. This mod therefore imports those files with **unknown FW** values. Assign the measured FW manually with **Manual Window + Manual Stamp**, or use a `.fwc` / NaNDL JSON file that already contains `frameWindow` values.

This is intentional: the mod does not invent a timing window from a replay timestamp.

## Marker colors

- FW1: red
- FW2: orange
- FW3-4: yellow
- FW5+: green
- Unknown: gray

## Precision

The calculator uses a Gaussian/erfc-style pass-probability model inspired by the public NaNDL precision calculator. It is an approximation and should not be treated as an official Geometry Dash difficulty score.
