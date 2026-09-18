/* SPDX-License-Identifier: MIT */
#include <zdse/radar.h>
#include <zmk/dongle_theme/raster.h>
#include <zmk/dongle_theme/theme.h>

static int width;
static int height;

static void mount(int w, int h, uint32_t now) {
  width = w;
  height = h;
  zdse_radar_reset(now);
}

static void gesture(int kind, uint32_t now) {
  switch (kind) {
  case DTE_TAP: zdse_radar_set_aurora(!zdse_radar_get_aurora()); break;
  case DTE_LEFT: zdse_radar_set_speed_profile_at(ZDSE_RADAR_SPEED_QUIET, now); break;
  case DTE_RIGHT: zdse_radar_set_speed_profile_at(ZDSE_RADAR_SPEED_ACTIVE, now); break;
  case DTE_UP: zdse_radar_set_targets(!zdse_radar_get_targets()); break;
  default: break;
  }
}

static int render(const struct dte_snapshot *snapshot, uint32_t now,
                  uint16_t *pixels) {
  (void)snapshot;
  dtr_begin(pixels, width, height);
  zdse_radar_set_boost_at(dte_touch_active(), now);
  struct zdse_radar_layout radar = zdse_radar_layout(width, height);
  zdse_radar_prepare_damage(&radar, width, height, now);
  zdse_radar_composite(&radar, width, height, now);
  return 1;
}

const struct dte_theme dte_selected_theme = {
    DTE_ABI_VERSION, "zdse-radar-phosphor", mount, gesture, render};
