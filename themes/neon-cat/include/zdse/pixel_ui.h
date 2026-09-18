/* SPDX-License-Identifier: MIT */
#pragma once
#include <stdint.h>
#include <zmk/dongle_theme/ui.h>

#define zdse_rect dte_ui_rect
#define zdse_round_rect dte_ui_round_rect
#define zdse_number dte_ui_format_number
#define zdse_percent dte_ui_format_percent
#define zdse_hash3 dte_ui_hash3

void zdse_panel(int x, int y, int w, int h, int r, int g, int b);
void zdse_battery(int x, int y, int w, int h, int value, int r, int g, int b);
/* style: 0=left/regular, 1=top/small, 2=right/regular. */
void zdse_battery_meter(int x,int y,int value,int style);
void zdse_button(const char *label, int x, int y, int w, int h, int active,
                 int pressed, int r, int g, int b);
