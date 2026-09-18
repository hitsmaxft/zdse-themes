/* SPDX-License-Identifier: MIT */
#include <zdse/neon_atlas_v2.h>
#include <zdse/neon_atlas_v2_data.h>
#include <zmk/dongle_theme/ui.h>
#include <zmk/dongle_theme/raster.h>

static int bit(const uint8_t *mask,int pixel){return !!(mask[pixel>>3]&(1u<<(7-(pixel&7))));}

void neon_v2_draw_scene(int x,int y){dte_sprite_blit_opaque(&neon_v2_scene,x,y);}
void neon_v2_draw_cat(int frame,int x,int y){
  if(frame<0)frame=0;if(frame>4)frame=4;dte_sprite_blit(&neon_v2_cats[frame],x,y,255);
}
void neon_v2_draw_heart(int x,int y){dte_sprite_blit_opaque(&neon_v2_heart,x,y);}

void neon_v2_draw_battery(enum neon_v2_side side,int value,int x,int y){
  const struct dte_sprite *base=side==NEON_V2_RIGHT?&neon_v2_battery_right:&neon_v2_battery_left;
  dte_sprite_blit_opaque(base,x,y);if(value<0)return;if(value>100)value=100;
  int limit=(47*value+50)/100;uint16_t color=dtr_rgb(value<20?240:value<40?255:84,value<20?40:value<40?195:250,value<20?55:value<40?50:104);
  for(int yy=0;yy<11;yy++)for(int xx=0;xx<limit;xx++)if(bit(neon_v2_battery_fill_mask,yy*47+xx))dtr_pixel565(x+5+xx,y+5+yy,color);
}
void neon_v2_draw_top_battery(int value,int x,int y){
  dte_sprite_blit(&neon_v2_top_battery,x,y,255);if(value<0)return;if(value>100)value=100;
  int limit=(31*value+50)/100;uint16_t color=dtr_rgb(value<20?240:value<40?255:84,value<20?40:value<40?195:250,value<20?55:value<40?50:104);
  for(int yy=0;yy<11;yy++)for(int xx=0;xx<limit;xx++)if(bit(neon_v2_top_fill_mask,yy*31+xx))dtr_pixel565(x+5+xx,y+5+yy,color);
}

static void indexed(const uint8_t *pixels,int w,int h,int x,int y,const uint16_t *palette){
  for(int yy=0;yy<h;yy++)for(int xx=0;xx<w;xx++){int index=pixels[yy*w+xx];if(index)dtr_pixel565(x+xx,y+yy,palette[index]);}
}

void neon_v2_draw_equalizer(enum neon_v2_side side,int frame,int x,int y){
  static uint16_t palette[4];palette[1]=dtr_rgb(129,48,230);palette[2]=dtr_rgb(205,36,228);palette[3]=dtr_rgb(246,4,184);
  if(frame<0)frame=0;frame&=3;const uint8_t *pixels=side==NEON_V2_RIGHT?neon_v2_eq_right[frame]:neon_v2_eq_left[frame];indexed(pixels,22,17,x,y,palette);
}

void neon_v2_draw_water(int frame,int x,int y){
  static uint16_t palette[4];palette[1]=dtr_rgb(255,220,72);palette[2]=dtr_rgb(246,48,184);palette[3]=dtr_rgb(145,48,225);
  if(frame<0)frame=0;indexed(neon_v2_water[frame&3],60,18,x,y,palette);
}
