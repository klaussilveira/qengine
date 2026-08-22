#!/usr/bin/env python3
import os
import sys

from PIL import Image, ImageDraw, ImageFont

import palette

ROOT = os.path.dirname(os.path.abspath(__file__))

CELL = 8
COLUMNS = 16
ROWS = 16

conchars = [
    "•╔═╗║ ║╚═╝ ■ ►••",
    "[]┌─┐│ │└─┘▬•├─┤",
    " !\"#$%&'()*+,-./",
    "0123456789:;<=>?",
    "@ABCDEFGHIJKLMNO",
    r"PQRSTUVWXYZ[\]^_",
    "'abcdefghijklmno",
    'pqrstuvwxyz{:}"◄',
]

slider_glyphs = {
    128: (
        "        ",
        "        ",
        "  XX    ",
        "  XXXXXX",
        "  XXXXXX",
        "  XX....",
        "   .....",
        "        ",
    ),
    129: (
        "        ",
        "        ",
        "        ",
        "XXXXXXXX",
        "XXXXXXXX",
        "........",
        "        ",
        "        ",
    ),
    130: (
        "        ",
        "        ",
        "    XX  ",
        "XXXXXX  ",
        "XXXXXX  ",
        "....XX  ",
        "     ...",
        "        ",
    ),
    131: (
        "        ",
        " ...... ",
        " .XXXX. ",
        " .XXXX. ",
        " .XXXX. ",
        " .XXXX. ",
        " ...... ",
        "  ....  ",
    ),
}


def plot(image, index, rows):
    x = (index % COLUMNS) * CELL
    y = (index // COLUMNS) * CELL

    for row, line in enumerate(rows):
        for column, cell in enumerate(line):
            if cell == "X":
                image.putpixel((x + column, y + row), palette.WHITE)
            elif cell == ".":
                image.putpixel((x + column, y + row), palette.BLACK)


def generate(assets):
    colors = palette.load(assets)

    image = Image.new("P", (COLUMNS * CELL, ROWS * CELL), palette.TRANSPARENT)
    image.putpalette(colors)
    draw = ImageDraw.Draw(image)
    font = ImageFont.truetype(
        os.path.join(ROOT, "fonts/int10h/Px437_AmstradPC1512.ttf"), CELL
    )

    for row, characters in enumerate(conchars):
        for column, character in enumerate(characters):
            x = column * CELL
            y = row * CELL
            draw.text((x, y + 1), character, font=font, fill=palette.BLACK)
            draw.text((x, y), character, font=font, fill=palette.WHITE)

    half = ROWS // 2 * CELL
    image.paste(image.crop((0, 0, COLUMNS * CELL, half)), (0, half))

    for index, rows in slider_glyphs.items():
        plot(image, index, rows)

    path = os.path.join(assets, "pics/conchars.pcx")
    image.save(path)
    print("conchars.pcx")


if __name__ == "__main__":
    if len(sys.argv) < 2:
        sys.exit("usage: fontgen.py <assets directory>")

    generate(sys.argv[1])
