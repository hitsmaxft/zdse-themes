#!/usr/bin/env python3
"""Behavior checks for the fixed-point phosphor radar compositor."""

import argparse
import ctypes as C
from pathlib import Path


def verify(folder: Path) -> None:
    library = "theme.dylib" if (folder / "theme.dylib").exists() else "theme.so"
    api = C.CDLL(str((folder / library).resolve()))
    api.dte_hash.restype = C.c_uint32
    api.dte_pixels.restype = C.POINTER(C.c_uint16)
    api.zdse_radar_angle_tenths.argtypes = [C.c_uint32]
    api.zdse_radar_angle_tenths.restype = C.c_int
    api.zdse_radar_phosphor_level.argtypes = [C.c_uint, C.c_uint, C.c_uint, C.c_uint32]
    api.zdse_radar_phosphor_level.restype = C.c_int
    api.zdse_radar_log_decay_alpha.argtypes = [C.c_uint, C.c_uint]
    api.zdse_radar_log_decay_alpha.restype = C.c_int
    api.zdse_radar_ring_radius.argtypes = [C.c_uint]
    api.zdse_radar_ring_radius.restype = C.c_int
    api.zdse_radar_last_pixels_shaded.restype = C.c_uint32
    api.zdse_radar_last_dirty_tiles.restype = C.c_uint32
    api.zdse_radar_get_boost.restype = C.c_int
    api.zdse_radar_revolution_ms.restype = C.c_uint32

    assert [api.zdse_radar_ring_radius(i) for i in range(1, 5)] == [25, 50, 75, 100]

    trails = {
        distance: [api.zdse_radar_log_decay_alpha(age, distance) for age in range(61)]
        for distance in (0, 25, 50, 75, 100)
    }
    for trail in trails.values():
        assert trail[0] == 255 and trail[60] == 0
        assert all(a >= b for a, b in zip(trail, trail[1:])), trail
    assert trails[0][10] != trails[100][10], "distance must alter log(n+s) decay"

    api.dte_init(280, 240)
    assert api.zdse_radar_get_aurora() == 0
    assert api.zdse_radar_angle_tenths(0) == 0
    assert api.zdse_radar_angle_tenths(1_000) == 360
    assert api.zdse_radar_angle_tenths(9_999) == 3_599
    assert api.zdse_radar_angle_tenths(10_000) == 0
    api.dte_init(280, 240)
    api.zdse_radar_set_speed_profile_at(1, 0)
    assert api.zdse_radar_angle_tenths(1_000) == 240
    assert api.zdse_radar_angle_tenths(14_999) == 3_599
    assert api.zdse_radar_angle_tenths(15_000) == 0

    api.dte_init(280, 240)
    assert api.zdse_radar_angle_tenths(2_000) == 720
    api.zdse_radar_set_speed_profile_at(1, 2_000)
    assert api.zdse_radar_angle_tenths(2_000) == 720
    assert api.zdse_radar_angle_tenths(3_000) == 960
    api.zdse_radar_set_boost_at(1, 3_000)
    assert api.zdse_radar_get_boost() == 1
    assert api.zdse_radar_angle_tenths(4_000) == 1_440
    api.zdse_radar_set_boost_at(0, 4_000)
    assert api.zdse_radar_angle_tenths(5_000) == 1_680

    # A live contact doubles speed only until release; left/right select slow/fast.
    api.dte_init(280, 240)
    api.dte_render(1_000)
    api.dte_touch(100, 100, 1, 1_000)
    api.dte_render(1_000)
    assert api.zdse_radar_get_boost() == 1
    api.dte_render(2_000)
    assert api.zdse_radar_angle_tenths(2_000) == 1_080
    api.dte_touch(100, 100, 0, 2_000)
    api.dte_render(2_000)
    assert api.zdse_radar_get_boost() == 0
    api.dte_render(3_000)
    assert api.zdse_radar_angle_tenths(3_000) == 1_440
    api.dte_gesture(2, 3_000)
    assert api.zdse_radar_get_speed_profile() == 1
    api.dte_gesture(3, 3_000)
    assert api.zdse_radar_get_speed_profile() == 0

    api.zdse_radar_set_aurora(0)
    phosphor = [api.zdse_radar_phosphor_level(age, 50, 900, age) for age in range(0, 2_001, 8)]
    assert phosphor[0] == 255 and phosphor[-1] == 0
    assert all(a >= b for a, b in zip(phosphor, phosphor[1:])), phosphor
    assert phosphor[0] - phosphor[13] > phosphor[81] - phosphor[94], "decay must be fast then slow"

    api.zdse_radar_set_aurora(1)
    modulated = api.zdse_radar_phosphor_level(700, 62, 1_370, 2_000)
    api.zdse_radar_set_aurora(0)
    plain = api.zdse_radar_phosphor_level(700, 62, 1_370, 2_000)
    assert abs(modulated - plain) <= 12, (modulated, plain)

    def initialize(aurora=1, targets=1, cold=0, now=0):
        api.dte_init(280, 240)
        api.zdse_radar_set_aurora(aurora)
        api.zdse_radar_set_targets(targets)
        api.zdse_radar_set_cold_start(cold, now)

    def full_frame(now: int, aurora=1, targets=1, cold=0) -> int:
        initialize(aurora, targets, cold)
        assert api.dte_render(now) == 1
        return api.dte_hash()

    assert full_frame(1_234) == full_frame(1_234), "same state and timestamp must be deterministic"
    assert full_frame(0, aurora=0, targets=0) != full_frame(1_000, aurora=0, targets=0)
    assert full_frame(0, aurora=0, targets=0) == full_frame(10_000, aurora=0, targets=0)
    assert full_frame(500, cold=1) != full_frame(500, cold=0)

    # Retained redraws must survive the 0-degree seam and arbitrary frame skips.
    for previous, current in ((9_990, 10_010), (3_000, 5_250), (29_900, 30_042)):
        initialize(aurora=1, targets=1)
        api.dte_render(previous)
        api.dte_render(current)
        skipped_hash = api.dte_hash()
        assert skipped_hash == full_frame(current), (previous, current)

    initialize(aurora=1, targets=1)
    api.dte_render(2_000)
    api.dte_render(2_042)
    retained_hash = api.dte_hash()
    shaded = api.zdse_radar_last_pixels_shaded()
    dirty_tiles = api.zdse_radar_last_dirty_tiles()
    assert 0 < shaded <= 17_000, shaded
    assert 0 < dirty_tiles < 70, dirty_tiles
    assert retained_hash == full_frame(2_042), "retained damage must equal a full repaint"

    # One complete active revolution at 24 fps must never expose tile edges.
    for frame in range(1, 241):
        previous = round((frame - 1) * 1_000 / 24)
        current = round(frame * 1_000 / 24)
        initialize(aurora=1, targets=1)
        api.dte_render(previous)
        api.dte_render(current)
        retained = api.dte_hash()
        assert retained == full_frame(current), (frame, previous, current)

    initialize(aurora=0, targets=1)
    api.dte_render(0)
    outer_beam = api.dte_pixels()[20 * 280 + 140]
    assert ((outer_beam >> 5) & 63) >= 55, "beam must meet the 100px outer ring"

    # First target is (-42,-28). Compare the same timestamp with targets off;
    # the brighter pixel must stay at that fixed world coordinate.
    target_angle10 = 3_037
    target_hit_ms = target_angle10 * api.zdse_radar_revolution_ms() // 3_600
    echo_time = target_hit_ms + 300
    initialize(aurora=0, targets=1)
    api.dte_render(echo_time)
    echo_pixel = api.dte_pixels()[(120 - 28) * 280 + (140 - 42)]
    initialize(aurora=0, targets=0)
    api.dte_render(echo_time)
    plain_pixel = api.dte_pixels()[(120 - 28) * 280 + (140 - 42)]
    assert echo_pixel != plain_pixel, "target echo must remain at its world coordinate"

    print(
        f"radar-phosphor: radial log trail, retained parity, fixed echoes PASS "
        f"({shaded} shaded pixels, {dirty_tiles} dirty tiles, 240-frame parity)"
    )


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("preview", type=Path)
    verify(parser.parse_args().preview)
