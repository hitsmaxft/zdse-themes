/* SPDX-License-Identifier: BSD-2-Clause */
#pragma once

/* Exact bitmap subsets from Spleen 2.2.0 BDF; no scaling or antialiasing. */
void zdse_spleen_wpm(int center_x, int y);
void zdse_spleen_number(int value, int center_x, int y);
void zdse_spleen_ui(const char *text, int x, int y, int scale, int centered,
                    int r, int g, int b);
