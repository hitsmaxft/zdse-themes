/* SPDX-License-Identifier: MIT */
#include <zdse/neon_animation.h>

struct neon_animation_frame neon_animation_resolve(int wpm,uint32_t now){
  struct neon_animation_frame out={.cat=0,.equalizer=0,.water=(now/250)%4};
  uint32_t phase=now%4200;
  if(phase>=3600&&phase<3740)out.cat=1;
  if(wpm>=80&&now%1800<720){static const uint8_t sequence[]={0,2,3,4,3,2};out.cat=sequence[(now%1800)/120];}
  if(wpm>0){uint32_t interval=wpm>=120?70:wpm>=80?100:wpm>=40?150:220;out.equalizer=(now/interval)%4;}
  return out;
}
