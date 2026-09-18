/* SPDX-License-Identifier: MIT */
#pragma once
#include <stdint.h>
struct neon_animation_frame { uint8_t cat,equalizer,water; };
struct neon_animation_frame neon_animation_resolve(int wpm,uint32_t now);
