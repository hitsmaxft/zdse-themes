# ZDSE Themes

Small theme examples for
[`zmk-dongle-screen-engine`](https://github.com/hitsmaxft/zmk-dongle-screen-engine).
They are published as visual and code references for authors building their own
ZMK dongle-screen themes.

## Neon Cat

A synthwave pixel HUD with a time-driven character, water shimmer, equalizer,
keyboard status, and modifier feedback.

![Neon Cat preview](docs/previews/neon-cat.png)

The public example contains the animation resolver only. Original artwork,
fonts, sprite sheets, and generated atlas payloads are intentionally private.

## Phosphor Radar

A procedural green CRT-style radar with a bright sweep, smooth fading trail,
stationary echoes, touch-to-boost, and swipe-selectable speed.

![Phosphor Radar preview](docs/previews/radar.png)

The complete procedural example is under [`examples/radar`](examples/radar).

## Writing a theme

Start with the Engine's
[`examples/minimal-theme`](https://github.com/hitsmaxft/zmk-dongle-screen-engine/tree/main/examples/minimal-theme),
then use these examples for animation timing, retained redraws, gestures, and
preview integration. Shared raster, sprite, and pixel-UI helpers belong in the
Engine; theme repositories should keep only their own motion, layout, palette,
and composition.

Preview images are native/WASM RGB565 render results, not physical-display
validation.

## License

Source code is MIT licensed. Preview images are provided for documentation and
visual reference only.
