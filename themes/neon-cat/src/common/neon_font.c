/* SPDX-License-Identifier: MIT */
#include <zdse/neon_font.h>
#include <zdse/neon_assets.h>
#include <zdse/neon_cat_layout.h>
#include <zmk/dongle_theme/raster.h>

static int face(const uint8_t *mask,int x,int y) {
  if(x<0||x>=NC_FONT_CELL_W||y<0||y>=NC_FONT_CELL_H)return 0;
  int pixel=y*NC_FONT_CELL_W+x;
  return !!(mask[pixel>>3]&(1u<<(7-(pixel&7))));
}

static void solid(const uint8_t *mask,int x,int y,uint16_t color) {
  for(int yy=0;yy<NC_FONT_CELL_H;yy++)for(int xx=0;xx<NC_FONT_CELL_W;xx++)
    if(face(mask,xx,yy))dtr_pixel565(x+xx,y+yy,color);
}

static void outline(const uint8_t *mask,int x,int y,uint16_t color) {
  for(int yy=0;yy<NC_FONT_CELL_H;yy++)for(int xx=0;xx<NC_FONT_CELL_W;xx++){
    int near=0;
    for(int dy=-1;dy<=1&&!near;dy++)for(int dx=-1;dx<=1;dx++)if(face(mask,xx+dx,yy+dy)){near=1;break;}
    if(near)dtr_pixel565(x+xx,y+yy,color);
  }
}

void zdse_neon_text(const char *text,int x,int y,int centered,
                    enum zdse_neon_font_style style,int alpha) {
  int count=0;while(text[count])count++;
  int total=count*NC_FONT_ADVANCE,pen=centered?x-total/2:x;
  for(int i=0;text[i];i++,pen+=NC_FONT_ADVANCE) {
    int c=(unsigned char)text[i];if(c>='a'&&c<='z')c-=32;
    if(c<'A'||c>'Z')continue;
    const uint8_t *mask=zdse_neon_font_masks[c-'A'];
    (void)alpha;
    if(style==ZDSE_NEON_FONT_BLACK){solid(mask,pen+2,y+2,dtr_rgb(170,160,184));outline(mask,pen+1,y+1,dtr_rgb(94,49,113));solid(mask,pen,y,dtr_rgb(18,17,24));}
    else{solid(mask,pen+2,y+2,dtr_rgb(100,30,143));outline(mask,pen+1,y+1,dtr_rgb(246,18,184));solid(mask,pen,y,dtr_rgb(248,248,251));}
  }
}

static int digit_bit(const uint8_t mask[34],int x,int y){
  int pixel=y*14+x;return !!(mask[pixel>>3]&(1u<<(7-(pixel&7))));
}

static void digit_layer(const uint8_t masks[10][34],int value,int x,int y,uint16_t color){
  for(int yy=0;yy<19;yy++)for(int xx=0;xx<14;xx++)if(digit_bit(masks[value],xx,yy))dtr_pixel565(x+xx,y+yy,color);
}

void zdse_neon_digits(const char *text,int center_x,int y){
  int count=0;while(text[count]>='0'&&text[count]<='9')count++;
  int pen=center_x-count*14/2;
  for(int i=0;i<count;i++,pen+=14)digit_layer(zdse_wpm_digit_face,text[i]-'0',pen,y,dtr_rgb(125,239,255));
}

void zdse_neon_number(int value,int center_x,int y){
  char text[4];int count=0;
  if(value>999)value=999;
  if(value>=100)text[count++]='0'+value/100;
  if(value>=10)text[count++]='0'+value/10%10;
  text[count++]='0'+value%10;
  text[count]=0;zdse_neon_digits(text,center_x,y);
}
