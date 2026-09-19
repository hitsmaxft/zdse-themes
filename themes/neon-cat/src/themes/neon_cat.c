/* SPDX-License-Identifier: MIT */
/* Neon Cat 1.0 layout and generated C payload composition. */
#include <stdint.h>
#include <zdse/neon_animation.h>
#include <zdse/neon_assets.h>
#include <zdse/neon_cat_layout.h>
#include <zdse/neon_font.h>
#include <zdse/neon_scene.h>
#include <zdse/neon_widgets.h>
#include <zdse/pixel_ui.h>
#include <zdse/spleen_font.h>
#include <zmk/dongle_theme/adapter.h>
#include <zmk/dongle_theme/raster.h>
#include <zmk/dongle_theme/theme.h>
#include <zmk/dongle_theme/ui.h>

static int W, H, palette;
static struct {
  int valid, palette, endpoint, profile, layer, battery_count, battery_dongle,
      battery_left, battery_right, wpm, modifiers;
  uint32_t layer_hash;
  struct neon_animation_frame animation;
} previous;

__attribute__((weak)) int zdse_neon_region_mask(void) { return NC_REGION_ALL; }

static void mount(int width, int height, uint32_t now) {
  (void)now;
  W = width;
  H = height;
  palette = 0;
  previous.valid = 0;
}

static void gesture(int kind, uint32_t now) {
  (void)now;
  if (kind == DTE_TAP || kind == DTE_LEFT || kind == DTE_RIGHT)
    palette ^= 1;
  else if (kind == DTE_LONG_PRESS)
    palette = 0;
}

static void bluetooth(int x, int y, int r, int g, int b) {
  dtr_line(x + 9, y, x + 9, y + 29, 2, r, g, b, 255);
  dtr_line(x + 9, y, x + 18, y + 8, 2, r, g, b, 255);
  dtr_line(x + 18, y + 8, x + 1, y + 23, 2, r, g, b, 255);
  dtr_line(x + 1, y + 6, x + 18, y + 23, 2, r, g, b, 255);
  dtr_line(x + 18, y + 23, x + 9, y + 31, 2, r, g, b, 255);
}

static void header(const struct dte_snapshot *s, int ox, int oy) {
  char profile[4] = {'#', '1', 0, 0}, percent[8], layer[5] = {0};
  const char *selected = s->layer_name[0] ? s->layer_name : "BASE";
  if (!palette && s->endpoint != 1 && s->profile == 0 &&
      s->battery_count == 3 && s->battery_dongle == 78) {
    dte_sprite_blit(&zdse_header_base, ox, oy, 255);
    zdse_neon_text(selected, ox + NC_LAYER_X + NC_LAYER_W / 2, oy + NC_LAYER_Y,
                   1, ZDSE_NEON_FONT_WHITE, 255);
  } else {
    int cyan = palette ? 96 : 18;
    bluetooth(ox + NC_BLE_ICON_X, oy + NC_BLE_ICON_Y, cyan, 172, 253);
    zdse_spleen_ui(s->endpoint == 1 ? "USB" : "BLE", ox + NC_BLE_TEXT_X,
                   oy + NC_BLE_TEXT_Y, 2, 0, 140, 242, 250);
    int p = s->profile + 1;
    if (p > 9)
      p = 9;
    profile[1] = '0' + p;
    zdse_spleen_ui(profile, ox + NC_PROFILE_X, oy + NC_PROFILE_Y, 2, 0, 222,
                   226, 246);
    zdse_rect(ox + NC_LAYER_LABEL_X, oy + NC_LAYER_LABEL_Y + 7, 25, 1, 246, 48,
              198, 220);
    zdse_rect(ox + NC_LAYER_LABEL_X + 75, oy + NC_LAYER_LABEL_Y + 7, 25, 1, 246,
              48, 198, 220);
    zdse_spleen_ui("LAYER", ox + NC_LAYER_LABEL_X + 50, oy + NC_LAYER_LABEL_Y,
                   1, 1, 246, 48, 198);
    int n = 0;
    while (selected[n] && n < 4) {
      layer[n] = selected[n];
      n++;
    }
    zdse_neon_text(layer, ox + NC_LAYER_X + NC_LAYER_W / 2, oy + NC_LAYER_Y, 1,
                   ZDSE_NEON_FONT_WHITE, 255);
  }
  if (s->battery_count == 3) {
    neon_widgets_draw_top_battery(s->battery_dongle, ox + NC_TOP_BATTERY_X,
                                  oy + NC_TOP_BATTERY_Y);
    zdse_percent(percent, s->battery_dongle);
    zdse_spleen_ui(percent, ox + NC_TOP_PERCENT_X + NC_TOP_PERCENT_W / 2,
                   oy + NC_TOP_PERCENT_Y, 2, 1, 226, 231, 247);
  }
}

static void status(const struct dte_snapshot *s, int ox, int oy, uint32_t now) {
  char value[8];
  dte_sprite_blit(&zdse_status_chrome, ox, oy + NC_STATUS_Y, 255);
  /* Pure pixel chrome; never crop the old WPM value/equalizer into this layer.
   */
  zdse_rect(ox + 94, oy + 154, 92, 1, 83, 35, 125, 255);
  zdse_rect(ox + 91, oy + 155, 3, 1, 83, 35, 125, 255);
  zdse_rect(ox + 186, oy + 155, 3, 1, 83, 35, 125, 255);
  zdse_rect(ox + 90, oy + 156, 1, 36, 83, 35, 125, 255);
  zdse_rect(ox + 189, oy + 156, 1, 36, 83, 35, 125, 255);
  zdse_rect(ox + 91, oy + 192, 3, 1, 83, 35, 125, 255);
  zdse_rect(ox + 186, oy + 192, 3, 1, 83, 35, 125, 255);
  zdse_rect(ox + 94, oy + 193, 92, 1, 83, 35, 125, 255);
  /* Approved upstream bitmap font; clear the old cropped label first. */
  zdse_rect(ox + 118, oy + 155, 44, 14, 10, 7, 27, 255);
  zdse_spleen_wpm(ox + 140, oy + NC_WPM_FONT_Y);
  zdse_percent(value, s->battery_left);
  zdse_spleen_ui(value, ox + NC_LEFT_STATUS_X + 39, oy + NC_LEFT_STATUS_Y, 2, 1,
                 230, 233, 247);
  neon_widgets_draw_battery(0, s->battery_left, ox + NC_LEFT_BATTERY_X,
                            oy + NC_LEFT_BATTERY_Y);
  zdse_number(value, s->wpm);
  zdse_spleen_number(s->wpm, ox + NC_WPM_VALUE_X + NC_WPM_VALUE_W / 2,
                     oy + NC_WPM_DIGITS_Y);
  struct neon_animation_frame animation = neon_animation_resolve(s->wpm, now);
  neon_widgets_draw_equalizers(s->wpm, animation.equalizer, ox + NC_EQ_LEFT_X,
                               ox + NC_EQ_RIGHT_X, oy + NC_EQ_LEFT_Y);
  zdse_percent(value, s->battery_right);
  zdse_spleen_ui(value, ox + NC_RIGHT_STATUS_X + 40, oy + NC_RIGHT_STATUS_Y, 2,
                 1, 230, 233, 247);
  neon_widgets_draw_battery(1, s->battery_right, ox + NC_RIGHT_BATTERY_X,
                            oy + NC_RIGHT_BATTERY_Y);
}

static void footer(const struct dte_snapshot *s, int ox, int oy, uint32_t now) {
  static const char *labels[4] = {"CTRL", "ALT", "GUI", "SHIFT"};
  /* One indicator represents both left and right variants of each modifier. */
  static const int bits[4] = {0x11, 0x44, 0x88, 0x22};
  static const int xs[4] = {NC_CTRL_X, NC_ALT_X, NC_GUI_X, NC_SHIFT_X};
  static const int ys[4] = {NC_CTRL_Y, NC_ALT_Y, NC_GUI_Y, NC_SHIFT_Y};
  static const int widths[4] = {NC_CTRL_W, NC_ALT_W, NC_GUI_W, NC_SHIFT_W};
  (void)now;
  dte_sprite_blit_opaque(&zdse_footer_strip, ox, oy + NC_FOOTER_Y);
  for (int i = 0; i < 4; i++) {
    int active = !!(s->modifiers & bits[i]);
    zdse_button(labels[i], ox + xs[i], oy + ys[i], widths[i], NC_CTRL_H, active,
                0, i == 1 ? 255 : 230, i == 1 ? 183 : 45, i == 1 ? 64 : 215);
  }
  dte_sprite_blit_opaque(&zdse_zmk_logo, ox + NC_LOGO_X, oy + NC_LOGO_Y);
}

static uint32_t text_hash(const char *text) {
  uint32_t hash = 2166136261u;
  for (int i = 0; text[i]; i++) {
    hash ^= (uint8_t)text[i];
    hash *= 16777619u;
  }
  return hash;
}
static int header_changed(const struct dte_snapshot *s) {
  return palette != previous.palette || s->endpoint != previous.endpoint ||
         s->profile != previous.profile || s->layer != previous.layer ||
         s->battery_count != previous.battery_count ||
         s->battery_dongle != previous.battery_dongle ||
         text_hash(s->layer_name) != previous.layer_hash;
}

static int render(const struct dte_snapshot *s, uint32_t now,
                  uint16_t *pixels) {
  dtr_begin(pixels, W, H);
  dtr_damage_begin();
  int ox = (W - NC_CANVAS_W) / 2, oy = (H - NC_CANVAS_H) / 2;
  int regions = zdse_neon_region_mask();
  struct neon_animation_frame animation = neon_animation_resolve(s->wpm, now);
  if (!previous.valid)
    dtr_damage_all();
  else {
    if ((regions & NC_REGION_SCENE) && animation.cat != previous.animation.cat)
      dtr_damage_rect(ox + 158, oy + 87, 100, 64);
    if ((regions & NC_REGION_SCENE) &&
        animation.water != previous.animation.water)
      dtr_damage_rect(ox + 110, oy + NC_SCENE_Y + 58, 60, 18);
    if ((regions & NC_REGION_STATUS) &&
        (s->wpm != previous.wpm ||
         animation.equalizer != previous.animation.equalizer))
      dtr_damage_rect(ox + 89, oy + 153, 102, 42);
    if ((regions & NC_REGION_STATUS) &&
        s->battery_left != previous.battery_left)
      dtr_damage_rect(ox + 16, oy + 152, 62, 40);
    if ((regions & NC_REGION_STATUS) &&
        s->battery_right != previous.battery_right)
      dtr_damage_rect(ox + 201, oy + 152, 65, 40);
    if ((regions & NC_REGION_HEADER) && header_changed(s))
      dtr_damage_rect(ox, oy, 280, 60);
    if ((regions & NC_REGION_FOOTER) && s->modifiers != previous.modifiers)
      dtr_damage_rect(ox, oy + 198, 280, 42);
  }
  dtr_clear(0, 0, 0);
  if (regions & NC_REGION_SCENE) {
    neon_scene_draw(ox, oy, &animation);
    if (regions == NC_REGION_SCENE)
      zdse_rect(ox, oy + NC_SCENE_Y, NC_SCENE_W, 11, 0, 0, 0, 255);
  }
  if (regions & NC_REGION_STATUS) {
    zdse_rect(ox, oy + NC_STATUS_Y, NC_CANVAS_W, NC_STATUS_H, 12, 6, 32, 255);
    status(s, ox, oy, now);
  }
  if (regions & NC_REGION_FOOTER) {
    zdse_rect(ox, oy + NC_FOOTER_Y, NC_CANVAS_W, NC_FOOTER_H, 12, 6, 32, 255);
    footer(s, ox, oy, now);
  }
  if (regions & NC_REGION_HEADER) {
    zdse_rect(ox, oy + NC_HEADER_Y, NC_CANVAS_W, NC_HEADER_H, 0, 0, 0, 255);
    header(s, ox, oy);
  }
  previous.valid = 1;
  previous.palette = palette;
  previous.endpoint = s->endpoint;
  previous.profile = s->profile;
  previous.layer = s->layer;
  previous.battery_count = s->battery_count;
  previous.battery_dongle = s->battery_dongle;
  previous.battery_left = s->battery_left;
  previous.battery_right = s->battery_right;
  previous.wpm = s->wpm;
  previous.modifiers = s->modifiers;
  previous.layer_hash = text_hash(s->layer_name);
  previous.animation = animation;
  return !!(regions & (NC_REGION_SCENE | NC_REGION_STATUS));
}

DTE_THEME_RASTER_ADAPTER("zdse-neon-cat", mount, gesture, render);
