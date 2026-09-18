/* SPDX-License-Identifier: MIT */
#include <zdse/radar.h>
#include <zdse/radar_luts.h>
#include <zmk/dongle_theme/raster.h>

#define AURORA_COUNT                                                           \
  (ZDSE_RADAR_AURORA_ANGLE_SAMPLES * ZDSE_RADAR_AURORA_RADIUS_SAMPLES)
#define AURORA_PERIOD_MS 143u
#define TARGET_DECAY_MS 1200u

struct radar_target { int8_t dx, dy; };
static const struct radar_target targets[] = {
    {-42, -28}, {31, -56}, {54, 19}, {-18, 62},
};

/* Original radial log(n+s) decay, retained because it gives the dark tail
 * materially smoother RGB565 level spacing than the time-only color ramp. */
static const uint16_t log_q10[] = {
    0,    0,    710,  1125, 1420, 1648, 1835, 1993, 2129, 2250, 2358, 2455,
    2545, 2627, 2702, 2773, 2839, 2901, 2960, 3015, 3068, 3118, 3165, 3211,
    3254, 3296, 3336, 3375, 3412, 3448, 3483, 3516, 3549, 3580, 3611, 3641,
    3670, 3698, 3725, 3751, 3777, 3803, 3827, 3851, 3875, 3898, 3921, 3943,
    3964, 3985, 4006, 4026, 4046, 4066, 4085, 4104, 4122, 4140, 4158, 4175,
    4193, 4210, 4226, 4243, 4259, 4275, 4290, 4306, 4321, 4336, 4350, 4365,
    4379, 4393, 4407, 4421, 4435, 4448, 4461, 4474, 4487, 4500, 4512, 4525,
    4537, 4549, 4561, 4573, 4585, 4596, 4608, 4619, 4630, 4641, 4652, 4663,
    4674, 4685, 4695, 4705, 4716, 4726, 4736, 4746, 4756, 4766, 4775, 4785,
    4795, 4804, 4813, 4823, 4832, 4841, 4850, 4859, 4868, 4876, 4885, 4894,
    4902, 4911, 4919, 4928, 4936, 4944, 4952, 4960, 4968, 4976, 4984, 4992,
    5000, 5008, 5015, 5023, 5031, 5038, 5046, 5053, 5060, 5068, 5075, 5082,
    5089, 5096, 5103, 5110, 5117, 5124, 5131, 5138, 5144, 5151, 5158, 5164,
    5171, 5178, 5184, 5191, 5197, 5203,
};

static uint8_t aurora_field[AURORA_COUNT];
static uint32_t aurora_epoch = UINT32_MAX;
static uint32_t cold_start_ms;
static uint32_t phase_time_ms;
static uint64_t phase_q16;
static uint32_t last_pixels, last_tiles;
static int speed_profile, boost_enabled, aurora_enabled, cold_start_enabled,
    targets_enabled;
static int damage_overlay, previous_head = -1, force_full;

static unsigned absolute(int value) {
  return value < 0 ? (unsigned)-value : (unsigned)value;
}

static void geometry(int dx, int dy, unsigned *radius, unsigned *angle10) {
  unsigned ax = absolute(dx), ay = absolute(dy);
  if (ax >= ZDSE_RADAR_LUT_SIDE || ay >= ZDSE_RADAR_LUT_SIDE) {
    *radius = 255;
    *angle10 = 0;
    return;
  }
  unsigned index = ay * ZDSE_RADAR_LUT_SIDE + ax;
  unsigned base = zdse_radar_quadrant_angle10[index];
  *radius = zdse_radar_quadrant_radius[index];
  if (dy <= 0)
    *angle10 = dx >= 0 ? base : (3600u - base) % 3600u;
  else
    *angle10 = dx >= 0 ? 1800u - base : 1800u + base;
}

static uint16_t add565(uint16_t base, uint16_t emission) {
  unsigned red = (base >> 11) + (emission >> 11);
  unsigned green = ((base >> 5) & 63) + ((emission >> 5) & 63);
  unsigned blue = (base & 31) + (emission & 31);
  if (red > 31) red = 31;
  if (green > 63) green = 63;
  if (blue > 31) blue = 31;
  return (uint16_t)((red << 11) | (green << 5) | blue);
}

static uint16_t blend_trail565(uint16_t base, int x, int y, int alpha) {
  static const uint8_t bayer[16] = {0, 8,  2, 10, 12, 4, 14, 6,
                                    3, 11, 1, 9,  15, 7, 13, 5};
  int red = (48 * alpha + ((base >> 11) * 255 / 31) * (255 - alpha)) / 255;
  int green = (244 * alpha + (((base >> 5) & 63) * 255 / 63) *
                                  (255 - alpha)) / 255;
  int blue = (122 * alpha + ((base & 31) * 255 / 31) * (255 - alpha)) / 255;
  int threshold = bayer[(y & 3) * 4 + (x & 3)];
  int r5 = (red * 31 * 16 / 255 + threshold) / 16;
  int g6 = (green * 63 * 16 / 255 + threshold) / 16;
  int b5 = (blue * 31 * 16 / 255 + threshold) / 16;
  if (r5 > 31) r5 = 31;
  if (g6 > 63) g6 = 63;
  if (b5 > 31) b5 = 31;
  return (uint16_t)((r5 << 11) | (g6 << 5) | b5);
}

int zdse_radar_log_decay_alpha(unsigned age_degrees,
                               unsigned distance_pixels) {
  if (age_degrees >= ZDSE_RADAR_TRAIL_DEGREES)
    return 0;
  if (distance_pixels > ZDSE_RADAR_RADIUS)
    distance_pixels = ZDSE_RADAR_RADIUS;
  unsigned start = distance_pixels + 1;
  unsigned current = start + age_degrees;
  unsigned end = start + ZDSE_RADAR_TRAIL_DEGREES;
  int span = log_q10[end] - log_q10[start];
  int remaining = log_q10[end] - log_q10[current];
  return (255 * remaining + span / 2) / span;
}

static int trig_tenths(int angle_tenths) {
  int degrees = angle_tenths / 10;
  int fraction = angle_tenths % 10;
  if (fraction < 0) {
    fraction += 10;
    degrees--;
  }
  return (dtr_trig(degrees) * (10 - fraction) +
          dtr_trig(degrees + 1) * fraction) /
         10;
}

uint32_t zdse_radar_revolution_ms(void) {
  uint32_t base = speed_profile == ZDSE_RADAR_SPEED_QUIET
                      ? ZDSE_RADAR_QUIET_REVOLUTION_MS
                      : ZDSE_RADAR_ACTIVE_REVOLUTION_MS;
  return boost_enabled ? base / 2u : base;
}

static uint64_t phase_at_q16(uint32_t now_ms) {
  const uint64_t turn_q16 = 3600ull << 16;
  uint32_t elapsed = now_ms - phase_time_ms;
  return (phase_q16 + (uint64_t)elapsed * turn_q16 /
                          zdse_radar_revolution_ms()) % turn_q16;
}

static void anchor_phase(uint32_t now_ms) {
  phase_q16 = phase_at_q16(now_ms);
  phase_time_ms = now_ms;
}

int zdse_radar_angle_tenths(uint32_t now_ms) {
  return (int)(phase_at_q16(now_ms) >> 16);
}

static void update_aurora(uint32_t now_ms) {
  uint32_t epoch = now_ms / AURORA_PERIOD_MS;
  if (epoch == aurora_epoch)
    return;
  aurora_epoch = epoch;
  int phase1 = (int)(epoch * 7u % 360u);
  int phase2 = (int)(epoch * 3u % 360u);
  for (int radius = 0; radius < ZDSE_RADAR_AURORA_RADIUS_SAMPLES; radius++)
    for (int angle = 0; angle < ZDSE_RADAR_AURORA_ANGLE_SAMPLES; angle++) {
      int first = dtr_trig(radius * 17 + angle * 11 - phase1);
      int second = dtr_trig(radius * 29 - angle * 7 + phase2);
      int value = 128 + first * 22 / 32767 + second * 14 / 32767;
      if (value < 0) value = 0;
      if (value > 255) value = 255;
      aurora_field[radius * ZDSE_RADAR_AURORA_ANGLE_SAMPLES + angle] =
          (uint8_t)value;
    }
}

static int aurora_sample(unsigned radius, unsigned angle10) {
  unsigned angle_q8 = angle10 * ZDSE_RADAR_AURORA_ANGLE_SAMPLES * 256u / 3600u;
  unsigned radius_q8 = radius * (ZDSE_RADAR_AURORA_RADIUS_SAMPLES - 1) * 256u /
                       ZDSE_RADAR_RADIUS;
  unsigned a0 = (angle_q8 >> 8) % ZDSE_RADAR_AURORA_ANGLE_SAMPLES;
  unsigned a1 = (a0 + 1) % ZDSE_RADAR_AURORA_ANGLE_SAMPLES;
  unsigned r0 = radius_q8 >> 8;
  unsigned r1 = r0 + 1 < ZDSE_RADAR_AURORA_RADIUS_SAMPLES ? r0 + 1 : r0;
  unsigned af = angle_q8 & 255, rf = radius_q8 & 255;
  int p00 = aurora_field[r0 * ZDSE_RADAR_AURORA_ANGLE_SAMPLES + a0];
  int p01 = aurora_field[r0 * ZDSE_RADAR_AURORA_ANGLE_SAMPLES + a1];
  int p10 = aurora_field[r1 * ZDSE_RADAR_AURORA_ANGLE_SAMPLES + a0];
  int p11 = aurora_field[r1 * ZDSE_RADAR_AURORA_ANGLE_SAMPLES + a1];
  int top = (p00 * (256 - (int)af) + p01 * (int)af) >> 8;
  int bottom = (p10 * (256 - (int)af) + p11 * (int)af) >> 8;
  return (top * (256 - (int)rf) + bottom * (int)rf) >> 8;
}

int zdse_radar_phosphor_level(unsigned age_ms, unsigned radius,
                              unsigned angle10, uint32_t now_ms) {
  if (age_ms >= ZDSE_RADAR_TRAIL_MS)
    return 0;
  unsigned index = (age_ms + ZDSE_RADAR_PHOSPHOR_STEP_MS / 2) /
                   ZDSE_RADAR_PHOSPHOR_STEP_MS;
  if (index >= ZDSE_RADAR_PHOSPHOR_SAMPLES)
    return 0;
  int fast = zdse_radar_phosphor_fast[index];
  int slow = zdse_radar_phosphor_slow[index];
  if (aurora_enabled && slow) {
    update_aurora(now_ms);
    int modulation = 256 + (aurora_sample(radius, angle10) - 128) * 12 / 128;
    slow = slow * modulation / 256;
  }
  int level = fast + slow;
  return level > 255 ? 255 : level;
}

static int angle_age(int head, unsigned angle10) {
  int age = head - (int)angle10;
  if (age < 0) age += 3600;
  return age;
}

static int target_level(int dx, int dy, uint32_t now_ms) {
  if (!targets_enabled)
    return 0;
  unsigned target_radius, target_angle;
  geometry(dx, dy, &target_radius, &target_angle);
  (void)target_radius;
  uint32_t revolution = zdse_radar_revolution_ms();
  int age10 = angle_age(zdse_radar_angle_tenths(now_ms), target_angle);
  uint32_t age = (uint64_t)(unsigned)age10 * revolution / 3600u;
  if (age >= TARGET_DECAY_MS)
    return 0;
  if (cold_start_enabled && age > now_ms - cold_start_ms)
    return 0;
  unsigned index = (age + ZDSE_RADAR_PHOSPHOR_STEP_MS / 2) /
                   ZDSE_RADAR_PHOSPHOR_STEP_MS;
  int level = zdse_radar_phosphor_fast[index] + zdse_radar_phosphor_slow[index];
  return level > 255 ? 255 : level;
}

void zdse_radar_reset(uint32_t now_ms) {
  speed_profile = ZDSE_RADAR_SPEED_ACTIVE;
  boost_enabled = 0;
  aurora_enabled = 0;
  cold_start_enabled = 0;
  targets_enabled = 1;
  damage_overlay = 0;
  cold_start_ms = now_ms;
  phase_time_ms = now_ms;
  phase_q16 = 0;
  aurora_epoch = UINT32_MAX;
  previous_head = -1;
  force_full = 1;
  last_pixels = last_tiles = 0;
}

static void mark_dynamic_tiles(const struct zdse_radar_layout *radar,
                               int width, int height, int head) {
  int tail = (int)ZDSE_RADAR_TRAIL_DEGREES * 10;
  for (int y = 0; y < height; y += 16)
    for (int x = 0; x < width; x += 16) {
      int right = x + 15 < width ? x + 15 : width - 1;
      int bottom = y + 15 < height ? y + 15 : height - 1;
      int nearest_x = radar->cx < x ? x - radar->cx
                      : radar->cx > right ? radar->cx - right : 0;
      int nearest_y = radar->cy < y ? y - radar->cy
                      : radar->cy > bottom ? radar->cy - bottom : 0;
      int limit = radar->radius + 5;
      if (nearest_x * nearest_x + nearest_y * nearest_y > limit * limit)
        continue;

      int sample_x = x + 7 - radar->cx, sample_y = y + 7 - radar->cy;
      if (sample_x < -ZDSE_RADAR_RADIUS) sample_x = -ZDSE_RADAR_RADIUS;
      if (sample_x > ZDSE_RADAR_RADIUS) sample_x = ZDSE_RADAR_RADIUS;
      if (sample_y < -ZDSE_RADAR_RADIUS) sample_y = -ZDSE_RADAR_RADIUS;
      if (sample_y > ZDSE_RADAR_RADIUS) sample_y = ZDSE_RADAR_RADIUS;
      unsigned radius, angle;
      geometry(sample_x, sample_y, &radius, &angle);
      int nearest = nearest_x > nearest_y ? nearest_x : nearest_y;
      int margin = nearest < 16 ? 1800 : 8500 / (int)radius + 35;
      int age = angle_age(head, angle);
      if (age <= tail + margin || age >= 3600 - margin)
        dtr_damage_rect(x, y, 16, 16);
    }
}

void zdse_radar_prepare_damage(const struct zdse_radar_layout *radar,
                               int width, int height, uint32_t now_ms) {
  int head = zdse_radar_angle_tenths(now_ms);
  dtr_damage_begin();
  if (force_full || previous_head < 0) {
    dtr_damage_all();
  } else {
    mark_dynamic_tiles(radar, width, height, head);
    mark_dynamic_tiles(radar, width, height, previous_head);
    if (targets_enabled)
      for (unsigned i = 0; i < sizeof(targets) / sizeof(targets[0]); i++)
        dtr_damage_rect(radar->cx + targets[i].dx - 7,
                        radar->cy + targets[i].dy - 7, 15, 15);
  }
  previous_head = head;
  force_full = 0;
  const uint32_t *dirty = dtr_dirty_tiles();
  last_tiles = 0;
  for (int row = 0; row < (height + 15) / 16; row++)
    for (int column = 0; column < (width + 15) / 16; column++)
      if (dirty[row] & (1u << column)) last_tiles++;
}

void zdse_radar_composite(const struct zdse_radar_layout *radar, int width,
                          int height, uint32_t now_ms) {
  const uint32_t *dirty = dtr_dirty_tiles();
  int head = zdse_radar_angle_tenths(now_ms);
  uint32_t revolution = zdse_radar_revolution_ms();
  int ux = trig_tenths(head), uy = -trig_tenths(head + 900);
  last_pixels = 0;
  if (aurora_enabled) update_aurora(now_ms);

  for (int y = 0; y < height; y++)
    for (int x = 0; x < width;) {
      int tile_end = ((x >> 4) + 1) << 4;
      if (tile_end > width) tile_end = width;
      if (!(dirty[y >> 4] & (1u << (x >> 4)))) {
        x = tile_end;
        continue;
      }
      for (; x < tile_end; x++) {
        int dx = x - radar->cx, dy = y - radar->cy;
        unsigned radius, angle;
        geometry(dx, dy, &radius, &angle);
        uint16_t color = zdse_radar_background_pixel(radius, angle);

        if (radius <= radar->radius) {
          int age10 = angle_age(head, angle);
          unsigned age_ms = (uint64_t)(unsigned)age10 * revolution / 3600u;
          if ((!cold_start_enabled || age_ms <= now_ms - cold_start_ms) &&
              age10 < (int)ZDSE_RADAR_TRAIL_DEGREES * 10) {
            int alpha = zdse_radar_log_decay_alpha((unsigned)age10 / 10u,
                                                    radius);
            if (aurora_enabled && alpha) {
              int modulation = 256 +
                  (aurora_sample(radius, angle) - 128) * 12 / 128;
              alpha = alpha * modulation / 256;
              if (alpha > 255) alpha = 255;
            }
            color = blend_trail565(color, x, y, alpha);
          }

          int along = dx * ux + dy * uy;
          unsigned across = absolute(dx * uy - dy * ux);
          unsigned distance_q8 = (across * 256u + 16383u) / 32767u;
          if (along >= -32767 && distance_q8 < 1152u) {
            int beam = distance_q8 <= 160u
                           ? 255
                           : (int)((1152u - distance_q8) * 210u / 992u);
            if (radius >= 97 && beam < 235) beam = 235;
            color = add565(color, zdse_radar_phosphor_rgb565[beam]);
          }

          if (targets_enabled)
            for (unsigned i = 0; i < sizeof(targets) / sizeof(targets[0]); i++) {
              int tx = dx - targets[i].dx, ty = dy - targets[i].dy;
              int distance2 = tx * tx + ty * ty;
              if (distance2 > 49) continue;
              int echo = target_level(targets[i].dx, targets[i].dy, now_ms);
              echo = echo * (49 - distance2) / 49;
              color = add565(color, zdse_radar_phosphor_rgb565[echo]);
            }
        }

        if (damage_overlay && ((x & 15) == 0 || (y & 15) == 0))
          color = add565(color, dtr_rgb(80, 18, 8));
        dtr_pixel565(x, y, color);
        last_pixels++;
      }
    }
}

void zdse_radar_set_speed_profile(int quiet) {
  zdse_radar_set_speed_profile_at(quiet, phase_time_ms);
}
void zdse_radar_set_speed_profile_at(int quiet, uint32_t now_ms) {
  int next = quiet ? ZDSE_RADAR_SPEED_QUIET : ZDSE_RADAR_SPEED_ACTIVE;
  if (next != speed_profile) {
    anchor_phase(now_ms);
    speed_profile = next;
    force_full = 1;
  }
}
int zdse_radar_get_speed_profile(void) { return speed_profile; }
void zdse_radar_set_boost_at(int enabled, uint32_t now_ms) {
  enabled = !!enabled;
  if (enabled != boost_enabled) {
    anchor_phase(now_ms);
    boost_enabled = enabled;
    force_full = 1;
  }
}
int zdse_radar_get_boost(void) { return boost_enabled; }
void zdse_radar_set_aurora(int enabled) {
  enabled = !!enabled;
  if (enabled != aurora_enabled) { aurora_enabled = enabled; force_full = 1; }
}
int zdse_radar_get_aurora(void) { return aurora_enabled; }
void zdse_radar_set_cold_start(int enabled, uint32_t now_ms) {
  enabled = !!enabled;
  if (enabled != cold_start_enabled) {
    cold_start_enabled = enabled;
    cold_start_ms = now_ms;
    force_full = 1;
  }
}
int zdse_radar_get_cold_start(void) { return cold_start_enabled; }
void zdse_radar_set_targets(int enabled) {
  enabled = !!enabled;
  if (enabled != targets_enabled) { targets_enabled = enabled; force_full = 1; }
}
int zdse_radar_get_targets(void) { return targets_enabled; }
void zdse_radar_set_damage_overlay(int enabled) {
  enabled = !!enabled;
  if (enabled != damage_overlay) { damage_overlay = enabled; force_full = 1; }
}
int zdse_radar_get_damage_overlay(void) { return damage_overlay; }
uint32_t zdse_radar_last_pixels_shaded(void) { return last_pixels; }
uint32_t zdse_radar_last_dirty_tiles(void) { return last_tiles; }
