/* SPDX-License-Identifier: MIT */
#include <zdse/neon_atlas_v2.h>
#include <zdse/neon_cat_layout.h>
#include <zdse/neon_scene.h>

void neon_scene_draw(int ox,int oy,const struct neon_animation_frame *frame){
  neon_v2_draw_scene(ox+NC_SCENE_X,oy+NC_SCENE_Y);
  neon_v2_draw_water(frame->water,ox+110,oy+NC_SCENE_Y+58);
  neon_v2_draw_cat(frame->cat,ox+NC_CAT_X,oy+NC_CAT_Y);
  neon_v2_draw_heart(ox+NC_HEART_X,oy+NC_HEART_Y);
}
