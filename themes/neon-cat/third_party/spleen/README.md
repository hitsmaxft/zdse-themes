# Spleen bitmap subset

- Upstream: https://github.com/fcambus/spleen
- Commit: `57f9219328c9f5873085320fe8bc8f7dd34b8791`
- Version in BDF metadata: 2.2.0
- License: BSD-2-Clause
- Imported glyphs: W/P/M from `spleen-8x16.bdf`; 0-9 from
  `spleen-6x12.bdf`; CTRL/ALT/GUI/SHIFT glyphs from `spleen-5x8.bdf`.
  Digits use exact 2×2 pixel replication at render time.

The bitmap rows in `src/common/spleen_font.c` are copied exactly from the BDF
glyph cells. The renderer performs no interpolation, outline generation,
resampling or antialiasing.
