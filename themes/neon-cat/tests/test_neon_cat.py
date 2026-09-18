#!/usr/bin/env python3
"""Behavior checks for the deterministic continuous neon-cat theme."""
import argparse
import ctypes as C
from pathlib import Path


def verify(folder: Path) -> None:
    api = C.CDLL(str((folder / "theme.dylib").resolve()))
    api.dte_hash.restype = C.c_uint32
    api.dte_pixels.restype = C.POINTER(C.c_uint16)
    api.dte_set_layer_name.argtypes = [C.c_char_p]

    def frame(now: int, modifiers: int = 0, gesture: int = 0,
              layer: int = 0, layer_name: bytes = b"BASE") -> int:
        api.dte_init(280, 240)
        api.dte_set_state(72, layer, 2, 0, modifiers, 87, 64, 93, 1, 1)
        api.dte_set_layer_name(layer_name)
        if gesture:
            api.dte_gesture(gesture, 0)
        assert api.dte_render(now) == 1
        return api.dte_hash()

    assert frame(800) == frame(800), "same timestamp must be deterministic"
    assert frame(100) != frame(800), "background and cat must animate"
    base=frame(100,0)
    for modifier in (0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80):
        assert frame(100,modifier)!=base,f"modifier 0x{modifier:02x} must change button"
    assert frame(100, layer=5, layer_name=b"NAVI") != frame(100), "layer snapshot must change header"
    frame(100);plain_footer=tuple(api.dte_pixels()[i] for i in range(198*280,240*280))
    frame(100,0,1);tap_footer=tuple(api.dte_pixels()[i] for i in range(198*280,240*280))
    assert tap_footer==plain_footer,"touch gestures must not fabricate modifier state"
    frame(100)
    pixels=api.dte_pixels()
    label=((226*31+127)//255<<11)|((231*63+127)//255<<5)|((247*31+127)//255)
    value=((125*31+127)//255<<11)|((239*63+127)//255<<5)|((255*31+127)//255)
    border=[]
    for x in range(90,190):border.extend((pixels[154*280+x],pixels[193*280+x]))
    for y in range(154,194):border.extend((pixels[y*280+90],pixels[y*280+189]))
    assert label not in border and value not in border,"WPM glyphs must not collide with panel border"
    api.dte_init(280,240);api.dte_set_battery_count(2)
    api.dte_set_state(38,0,2,0,0,75,100,-1,1,1);api.dte_render(100)
    pixels=api.dte_pixels()
    assert all(pixels[y*280+x]==0 for y in range(7,49) for x in range(217,270)), \
        "two-battery layout must leave dongle battery region empty"
    print("neon-cat: deterministic animation and button feedback PASS")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("preview", type=Path)
    verify(parser.parse_args().preview)
