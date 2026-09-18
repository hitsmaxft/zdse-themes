/* SPDX-License-Identifier: MIT */
#include <zdse/radar.h>
#include <zmk/dongle_theme/raster.h>

struct zdse_radar_layout zdse_radar_layout(int width, int height) {
  return (struct zdse_radar_layout){
      .cx = width / 2,
      .cy = height / 2,
      .radius = ZDSE_RADAR_RADIUS,
  };
}

int zdse_radar_ring_radius(unsigned ring_index) {
  if (ring_index > ZDSE_RADAR_RING_DIVISIONS)
    ring_index = ZDSE_RADAR_RING_DIVISIONS;
  return ZDSE_RADAR_RADIUS * (int)ring_index / ZDSE_RADAR_RING_DIVISIONS;
}

static unsigned nearest_period(unsigned value, unsigned period) {
  unsigned remainder = value % period;
  return remainder < period - remainder ? remainder : period - remainder;
}

uint16_t zdse_radar_background_pixel(unsigned radius, unsigned angle_tenths) {
  if (radius > ZDSE_RADAR_RADIUS)
    return dtr_rgb(1, 7, 5);

  int red = 2, green = 15, blue = 11;
  if (radius == 25 || radius == 50 || radius == 75) {
    red = 5;
    green = 50;
    blue = 31;
  }
  if (nearest_period(angle_tenths, 450) <= 8 && radius > 3 && radius < 97) {
    red = 4;
    green = 43;
    blue = 27;
  }
  if (radius >= 99) {
    red = 8;
    green = 92;
    blue = 49;
  }
  if (radius >= 95 && nearest_period(angle_tenths, 100) <= 10) {
    red = 12;
    green = 119;
    blue = 62;
  }
  return dtr_rgb(red, green, blue);
}
