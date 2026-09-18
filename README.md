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

## Basic ZMK demo build

Add the Engine and this repository to your ZMK `west.yml` manifest:

```yaml
manifest:
  remotes:
    - name: hitsmaxft
      url-base: https://github.com/hitsmaxft
  projects:
    - name: zmk-dongle-screen-engine
      remote: hitsmaxft
      revision: 4e6d2422b8fdc40e39473c4f07b106f5e2590928
      path: modules/zmk-dongle-screen-engine
    - name: zdse-themes
      remote: hitsmaxft
      revision: main
      path: modules/zdse-themes
```

Then add the Engine host shield and your display hardware shield to a dongle
target. This minimal `build.yaml` example enables the public Radar theme:

```yaml
include:
  - board: xiao_ble/nrf52840/zmk
    shield: <your-display-shield> dongle_screen_host
    cmake-args: >-
      -DCONFIG_ZDSE_RADAR_THEME=y
      -DCONFIG_ZMK_DONGLE_SCREEN_DIRECT_RGB565=y
      -DCONFIG_ZMK_DONGLE_SCREEN_FPS=24
    artifact-name: zdse_radar_demo
```

The display shield must configure its controller, resolution, rotation,
backlight, and optional touch device. The validated reference target uses an
nRF52840, a 280×240 ST7789 RGB565 display, and a CST816S touch controller.
Build success does not by itself prove physical display timing or touch wiring.

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
