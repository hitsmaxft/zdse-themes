/* SPDX-License-Identifier: MIT */
#pragma once
#include <stdint.h>

enum neon_v2_side { NEON_V2_LEFT=0, NEON_V2_RIGHT=1 };

void neon_v2_draw_scene(int x,int y);
void neon_v2_draw_cat(int frame,int x,int y);
void neon_v2_draw_heart(int x,int y);
void neon_v2_draw_battery(enum neon_v2_side side,int value,int x,int y);
void neon_v2_draw_top_battery(int value,int x,int y);
void neon_v2_draw_equalizer(enum neon_v2_side side,int frame,int x,int y);
void neon_v2_draw_water(int frame,int x,int y);
