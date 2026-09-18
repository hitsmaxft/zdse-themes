# Neon Cat 1.0

A complete synthwave RGB565 theme for `zmk-dongle-screen-engine` ABI 1.1.
It includes the renderer and generated C atlas/font payloads. Original artwork,
sprite sheets, and asset-generation tools are not distributed.

Firmware builds must opt in with:

```text
CONFIG_ZDSE_NEON_CAT_THEME=y
```

Merely adding this repository to a West workspace does not compile the theme.

Build a native/WASM preview from a workspace containing the Engine:

```sh
python modules/zmk-dongle-screen-engine/scripts/build_preview.py \
  --lvgl modules/modules/lib/gui/lvgl \
  --theme modules/zdse-themes/themes/neon-cat \
  --output build/neon-cat
```
