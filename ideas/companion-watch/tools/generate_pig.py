#!/usr/bin/env python3
"""
Generate pig sprite C header from walking sprite sheet.
- Walk: extracted from the sprite sheet (top row, 4 frames)
- Idle: gentle bob from frame 0
- Sleep: darkened + blue tint from frame 0
"""

from PIL import Image, ImageDraw, ImageEnhance
from pathlib import Path

SIZE = 32
GRID_COLS = 4
GRID_ROWS = 2


def rgb565_be(r, g, b):
    c = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)
    return ((c >> 8) & 0xFF) | ((c & 0xFF) << 8)


def load_walk_frames(path):
    img = Image.open(path).convert("RGBA")
    fw = img.width // GRID_COLS
    fh = img.height // GRID_ROWS
    frames = []
    for col in range(GRID_COLS):
        frame = img.crop((col * fw, 0, (col + 1) * fw, fh))
        frame = frame.resize((SIZE, SIZE), Image.LANCZOS)
        frames.append(frame)
    return frames


def create_idle_frames(base):
    offsets = [0, -1, 0, 1]
    frames = []
    bg = sample_bg_color(base)
    for off in offsets:
        f = Image.new("RGBA", (SIZE, SIZE), bg)
        f.paste(base, (0, off), base)
        frames.append(f)
    return frames


def create_sleep_frames(base):
    dark = ImageEnhance.Brightness(base).enhance(0.45)
    blue = Image.new("RGBA", (SIZE, SIZE), (0, 0, 50, 70))
    dark = Image.alpha_composite(dark.convert("RGBA"), blue)

    frames = []
    z_positions = [(22, 3), (23, 2), (24, 1), (23, 2)]
    for i, (zx, zy) in enumerate(z_positions):
        f = dark.copy()
        draw = ImageDraw.Draw(f)
        if i != 2:
            for dx, dy in [(0, 0), (0, 1), (1, 0), (1, 1)]:
                draw.point((zx + dx, zy + dy), fill=(180, 180, 255, 220))
            for dx, dy in [(0, 0), (0, 1), (1, 0), (1, 1)]:
                draw.point((zx + 3 + dx, zy - 2 + dy), fill=(150, 150, 255, 180))
        frames.append(f)
    return frames


def sample_bg_color(img):
    r, g, b, a = img.getpixel((0, 0))
    return (r, g, b, 255)


def frames_to_rgb565(frames):
    data = []
    for frame in frames:
        rgb = frame.convert("RGB")
        for y in range(SIZE):
            for x in range(SIZE):
                r, g, b = rgb.getpixel((x, y))
                data.append(rgb565_be(r, g, b))
    return data


def write_header(animations, output_path):
    with open(output_path, "w") as f:
        f.write("#pragma once\n#include <stdint.h>\n")
        f.write('#include "sprite_engine.h"\n\n')

        for name, data, fc, delay in animations:
            f.write(f"static const uint16_t {name}_frames[{len(data)}] = {{\n")
            for fi in range(fc):
                f.write(f"    // Frame {fi}\n")
                off = fi * SIZE * SIZE
                for y in range(SIZE):
                    f.write("    ")
                    for x in range(SIZE):
                        f.write(f"0x{data[off + y * SIZE + x]:04X},")
                    f.write("\n")
            f.write("};\n\n")

        f.write("static const sprite_animation_t pig_animations[] = {\n")
        for name, _, fc, delay in animations:
            f.write(
                f"    {{ .frames = {name}_frames, .frame_count = {fc},"
                f' .frame_delay_ms = {delay}, .name = "{name}" }},\n'
            )
        f.write("};\n\n")
        f.write(f"#define PIG_ANIMATION_COUNT {len(animations)}\n")


def main():
    src = Path(r"C:\Users\anton\Downloads\Minecraft_pig_walkin...-888999626-0.png")
    out = Path(r"C:\Users\anton\Desktop\projects\experimenting\ideas\companion-watch\firmware\main\pig_sprites.h")

    print(f"Loading {src}...")
    walk = load_walk_frames(src)
    print(f"  Walk: {len(walk)} frames")

    idle = create_idle_frames(walk[0])
    print(f"  Idle: {len(idle)} frames (bob from walk frame 0)")

    sleep = create_sleep_frames(walk[0])
    print(f"  Sleep: {len(sleep)} frames (darkened + zzz)")

    animations = [
        ("idle", frames_to_rgb565(idle), len(idle), 300),
        ("walk", frames_to_rgb565(walk), len(walk), 150),
        ("sleep", frames_to_rgb565(sleep), len(sleep), 500),
    ]

    write_header(animations, out)
    print(f"\nWritten to {out}")
    total_bytes = sum(len(d) * 2 for _, d, _, _ in animations)
    print(f"Total sprite data: {total_bytes:,} bytes ({total_bytes / 1024:.1f} KB)")


if __name__ == "__main__":
    main()
