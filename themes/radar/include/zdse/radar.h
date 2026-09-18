/* SPDX-License-Identifier: MIT */
#pragma once

#include <stdint.h>

#define ZDSE_RADAR_DIAMETER 200
#define ZDSE_RADAR_RADIUS (ZDSE_RADAR_DIAMETER / 2)
#define ZDSE_RADAR_RING_DIVISIONS 4
#define ZDSE_RADAR_ACTIVE_REVOLUTION_MS 10000u
#define ZDSE_RADAR_QUIET_REVOLUTION_MS 15000u
#define ZDSE_RADAR_TRAIL_MS 1667u
#define ZDSE_RADAR_TRAIL_DEGREES 60u
#define ZDSE_RADAR_AURORA_ANGLE_SAMPLES 64
#define ZDSE_RADAR_AURORA_RADIUS_SAMPLES 32

enum zdse_radar_speed_profile {
  ZDSE_RADAR_SPEED_ACTIVE = 0,
  ZDSE_RADAR_SPEED_QUIET = 1,
};

struct zdse_radar_layout {
  int cx;
  int cy;
  int radius;
};

struct zdse_radar_layout zdse_radar_layout(int width, int height);
int zdse_radar_ring_radius(unsigned ring_index);
uint16_t zdse_radar_background_pixel(unsigned radius, unsigned angle_tenths);

void zdse_radar_reset(uint32_t now_ms);
void zdse_radar_prepare_damage(const struct zdse_radar_layout *layout,
                               int width, int height, uint32_t now_ms);
void zdse_radar_composite(const struct zdse_radar_layout *layout, int width,
                          int height, uint32_t now_ms);

int zdse_radar_angle_tenths(uint32_t now_ms);
uint32_t zdse_radar_revolution_ms(void);
int zdse_radar_phosphor_level(unsigned age_ms, unsigned radius,
                              unsigned angle_tenths, uint32_t now_ms);
int zdse_radar_log_decay_alpha(unsigned age_degrees,
                               unsigned distance_pixels);

void zdse_radar_set_speed_profile(int quiet);
void zdse_radar_set_speed_profile_at(int quiet, uint32_t now_ms);
int zdse_radar_get_speed_profile(void);
void zdse_radar_set_boost_at(int enabled, uint32_t now_ms);
int zdse_radar_get_boost(void);
void zdse_radar_set_aurora(int enabled);
int zdse_radar_get_aurora(void);
void zdse_radar_set_cold_start(int enabled, uint32_t now_ms);
int zdse_radar_get_cold_start(void);
void zdse_radar_set_targets(int enabled);
int zdse_radar_get_targets(void);
void zdse_radar_set_damage_overlay(int enabled);
int zdse_radar_get_damage_overlay(void);
uint32_t zdse_radar_last_pixels_shaded(void);
uint32_t zdse_radar_last_dirty_tiles(void);
