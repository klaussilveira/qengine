#!/usr/bin/env python3
import sys
from PIL import Image, ImageFont, ImageDraw

# Setup game palette
game_palette = Image.open(sys.argv[1] + "/pics/colormap.pcx")

# Color palette indexes
black = 0
white = 15
transparent = 255
green = 208

conchars = [
    "•╔═╗║ ║╚═╝ ■ ►••",
    "[]┌─┐│ │└─┘▬•├─┤",
    " !\"#$%&'()*+,-./",
    "0123456789:;<=>?",
    "@ABCDEFGHIJKLMNO",
    "PQRSTUVWXYZ[\]^_",
    "'abcdefghijklmno",
    "pqrstuvwxyz{:}\"◄",
    "                "
]

conchars_selected = [
    "[]012345678─■►••",
    " !\"#$%&'()*+,-./",
    "0123456789:;<=>?",
    "@ABCDEFGHIJKLMNO",
    "PQRSTUVWXYZ[\]^_",
    "'abcdefghijklmno",
    "pqrstuvwxyz{:}\"◄"
]

image = Image.new("P", (128, 128), transparent)
image.putpalette(game_palette.palette)
draw = ImageDraw.Draw(image)
font = ImageFont.truetype("fonts/int10h/Px437_AmstradPC1512.ttf", 8)

# Draw base
row = 0
for characters in conchars:
    column = 0

    for char in range(0, len(characters)):
        draw.text((column, row + 1), characters[char], font=font, fill=black)
        draw.text((column, row), characters[char], font=font, fill=white)
        column += 8

    row += 8

# Draw selected
for characters in conchars_selected:
    column = 0

    for char in range(0, len(characters)):
        draw.text((column, row + 1), characters[char], font=font, fill=black)
        draw.text((column, row), characters[char], font=font, fill=white)
        column += 8

    row += 8

image.save(sys.argv[1] + "/pics/conchars.pcx")
