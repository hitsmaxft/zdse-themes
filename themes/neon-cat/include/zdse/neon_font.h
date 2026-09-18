/* SPDX-License-Identifier: MIT */
#pragma once

/* Theme-local A-Z pixel font derived from the visual grammar of the supplied
 * asset sheet; it is not part of the engine font or locale contract. */
enum zdse_neon_font_style { ZDSE_NEON_FONT_WHITE = 0, ZDSE_NEON_FONT_BLACK = 1 };
void zdse_neon_text(const char *text, int x, int y, int centered,
                    enum zdse_neon_font_style style, int alpha);
void zdse_neon_number(int value, int center_x, int y);
void zdse_neon_digits(const char *text, int center_x, int y);
