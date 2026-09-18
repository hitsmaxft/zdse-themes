#!/usr/bin/env python3
"""Compile and verify the public, asset-free Neon Cat animation resolver."""

import ctypes as C
from pathlib import Path
import subprocess
import tempfile


class Frame(C.Structure):
    _fields_ = [("cat", C.c_uint8), ("equalizer", C.c_uint8), ("water", C.c_uint8)]


root = Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory() as folder:
    library = Path(folder) / "neon-animation.dylib"
    subprocess.run([
        "clang", "-dynamiclib", "-I", str(root / "include"),
        str(root / "src/neon_animation.c"), "-o", str(library),
    ], check=True)
    api = C.CDLL(str(library))
    api.neon_animation_resolve.argtypes = [C.c_int, C.c_uint32]
    api.neon_animation_resolve.restype = Frame
    samples = [(0, 0), (72, 500), (128, 730), (92, 3_250), (0, 3_650)]
    first = [(f.cat, f.equalizer, f.water)
             for wpm, now in samples for f in [api.neon_animation_resolve(wpm, now)]]
    second = [(f.cat, f.equalizer, f.water)
              for wpm, now in samples for f in [api.neon_animation_resolve(wpm, now)]]
    assert first == second
    assert len(set(first)) > 1
    print({"samples": len(samples), "deterministic": True, "asset_free": True})
