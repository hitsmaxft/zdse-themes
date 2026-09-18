/* SPDX-License-Identifier: MIT */
#include <zdse/neon_atlas_v2.h>
#include <zdse/neon_widgets.h>

void neon_widgets_draw_battery(int right,int value,int x,int y){neon_v2_draw_battery(right?NEON_V2_RIGHT:NEON_V2_LEFT,value,x,y);}
void neon_widgets_draw_top_battery(int value,int x,int y){neon_v2_draw_top_battery(value,x,y);}
void neon_widgets_draw_equalizers(int wpm,int frame,int left_x,int right_x,int y){
  (void)wpm;neon_v2_draw_equalizer(NEON_V2_LEFT,frame,left_x,y);neon_v2_draw_equalizer(NEON_V2_RIGHT,frame,right_x,y);
}
