/* SPDX-License-Identifier: MIT */
#include <zdse/pixel_ui.h>
#include <zdse/neon_assets.h>
#include <zmk/dongle_theme/raster.h>

void zdse_panel(int x, int y, int w, int h, int r, int g, int b) {
  zdse_rect(x + 2, y + 2, w, h, 22, 4, 38, 230);
  zdse_rect(x, y, w, h, 10, 7, 27, 245);
  zdse_rect(x, y, w, 1, r, g, b, 230);
  zdse_rect(x, y, 1, h, r, g, b, 180);
  zdse_rect(x + w - 1, y + 3, 1, h - 3, r / 2, g / 2, b / 2, 180);
  zdse_rect(x + 3, y + h - 1, w - 3, 1, r / 2, g / 2, b / 2, 180);
}

void zdse_battery(int x, int y, int w, int h, int value, int r, int g, int b) {
  zdse_rect(x, y, w, h, 228, 231, 247, 255);
  zdse_rect(x + 2, y + 2, w - 4, h - 4, 25, 16, 47, 255);
  zdse_rect(x + w, y + h / 3, 2, h / 3 + 1, 228, 231, 247, 255);
  if (value >= 0) {
    int fill = (w - 6) * value / 100;
    if (fill > 0) zdse_rect(x + 3, y + 3, fill, h - 6, r, g, b, 255);
  }
}

void zdse_battery_meter(int x,int y,int value,int style) {
  int small=style==1;
  if(!small){
    int r=value<20?240:value<40?255:84,g=value<20?40:value<40?195:250,b=value<20?55:value<40?50:104;
    zdse_battery(x,y,54,19,value,r,g,b);return;
  }
  int fx=small?8:4,fy=small?5:4,fw=small?23:45,fh=11;
  zdse_rect(x+fx,y+fy,fw,fh,52,48,85,255);
  if(value>=0){
    int fill=fw*(value>100?100:value)/100;
    int r=value<20?240:value<40?255:84,g=value<20?40:value<40?195:250,b=value<20?55:value<40?50:104;
    uint16_t color=dtr_rgb(r,g,b);
    for(int yy=0;yy<fh;yy++){
      int bevel=yy==0||yy==fh-1;
      for(int xx=bevel;xx<fill-bevel;xx++)dtr_pixel565(x+fx+xx,y+fy+yy,color);
    }
  }
  dte_sprite_blit(&zdse_battery_small_frame,x,y,255);
}

static int mask_bit(const uint8_t *mask,int pixel){return !!(mask[pixel>>3]&(1u<<(7-(pixel&7))));}
static void mask_draw(const uint8_t *mask,int w,int h,int x,int y,int r,int g,int b){
  uint16_t color=dtr_rgb(r,g,b);
  for(int yy=0;yy<h;yy++)for(int xx=0;xx<w;xx++)if(mask_bit(mask,yy*w+xx))dtr_pixel565(x+xx,y+yy,color);
}
static void mask_border_draw(const uint8_t *outer,const uint8_t *inner,int w,int h,int x,int y,int r,int g,int b){
  uint16_t color=dtr_rgb(r,g,b);
  for(int yy=0;yy<h;yy++)for(int xx=0;xx<w;xx++){
    int pixel=yy*w+xx;
    if(mask_bit(outer,pixel)&&!mask_bit(inner,pixel))dtr_pixel565(x+xx,y+yy,color);
  }
}

void zdse_button(const char *label, int x, int y, int w, int h, int active,
                 int pressed, int r, int g, int b) {
  (void)r;(void)g;(void)b;
  int lift=active&&!pressed,fx=x-lift,fy=y-lift;
  const uint8_t *outer=w==34?zdse_button34_outer:zdse_button33_outer;
  const uint8_t *inner=w==34?zdse_button34_inner:zdse_button33_inner;
  mask_draw(outer,w,h,x+2,y+2,active?94:18,active?62:7,active?42:37);
  int br=active?245:90,bg=active?205:73,bb=active?125:122;
  mask_draw(outer,w,h,fx,fy,br,bg,bb);
  mask_draw(inner,w,h,fx,fy,active?32:18,active?10:7,active?27:37);
  const uint8_t *label_mask=label[0]=='C'?zdse_button_ctrl_label:
    label[0]=='A'?zdse_button_alt_label:label[0]=='G'?zdse_button_gui_label:zdse_button_shift_label;
  mask_draw(label_mask,w,h,fx,fy,active?245:90,active?205:73,active?125:122);
  mask_border_draw(outer,inner,w,h,fx,fy,br,bg,bb);
}
