#!/usr/bin/env python3
"""Generate the Windows VisiForm icon from the authoritative PNG source.

Requires:
    Pillow
"""

from __future__ import annotations

import sys
from pathlib import Path

from PIL import Image, ImageOps


REPO_ROOT = Path(__file__).resolve().parents[1]
SOURCE_ICON = REPO_ROOT / "assets" / "source" / "VisiFormIcon.png"
OUTPUT_ICON = REPO_ROOT / "assets" / "icons" / "windows" / "VisiForm.ico"
ICON_SIZES = [(16, 16), (20, 20), (24, 24), (32, 32), (40, 40), (48, 48), (64, 64), (128, 128), (256, 256)]
MASTER_SIZE = (256, 256)
SAFE_SCALE = 0.92


def build_padded_master(source: Image.Image) -> Image.Image:
    source_rgba = source.convert("RGBA")
    safe_dimension = int(round(MASTER_SIZE[0] * SAFE_SCALE))
    contained = ImageOps.contain(source_rgba, (safe_dimension, safe_dimension), Image.Resampling.LANCZOS)

    master = Image.new("RGBA", MASTER_SIZE, (0, 0, 0, 0))
    offset_x = (MASTER_SIZE[0] - contained.width) // 2
    offset_y = (MASTER_SIZE[1] - contained.height) // 2
    master.paste(contained, (offset_x, offset_y), contained)
    return master


def generate_icon() -> int:
    if not SOURCE_ICON.is_file():
        print(f"Source icon not found: {SOURCE_ICON}", file=sys.stderr)
        return 1

    OUTPUT_ICON.parent.mkdir(parents=True, exist_ok=True)

    try:
        with Image.open(SOURCE_ICON) as source_image:
            padded_master = build_padded_master(source_image)
            padded_master.save(OUTPUT_ICON, format="ICO", sizes=ICON_SIZES)
    except OSError as exc:
        print(f"Failed to generate Windows icon: {exc}", file=sys.stderr)
        return 1

    print(f"Wrote {OUTPUT_ICON}")
    return 0


if __name__ == "__main__":
    raise SystemExit(generate_icon())
