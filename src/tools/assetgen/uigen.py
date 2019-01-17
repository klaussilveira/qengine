#!/usr/bin/env python3
import sys
from PIL import Image, ImageFont, ImageDraw

# Setup game palette
game_palette = Image.open(sys.argv[1] + "/pics/colormap.pcx")

# Color palette indexes
black = 0
white = 15
transparent = 255
gold = 56
olive = 214
green = 208

def draw_character(character, group, color, shadow_color, character_name = None):
    if not character_name:
        character_name = character

    image = Image.new("P", (16, 24), transparent)
    image.putpalette(game_palette.palette)
    draw = ImageDraw.Draw(image)
    font = ImageFont.truetype("fonts/kenney/Kenney Future.ttf", 24)
    draw.text((1, -3), character, font=font, fill=shadow_color)
    draw.text((0, -4), character, font=font, fill=color)
    image.save(sys.argv[1] + "/pics/" + group +"_" + character_name + ".pcx")

# Generate health HUD numbers
for number in range(0, 10):
    draw_character(str(number), "num", white, black)

# Generate ammo HUD numbers
for number in range(0, 10):
    draw_character(str(number), "anum", olive, black)

# Generate health and ammo operators
draw_character("-", "num", white, black, "minus")
draw_character("-", "anum", olive, black, "minus")

# Generate animated cursor
for number in range(0, 16):
    image = Image.new("P", (22, 29), transparent)
    image.putpalette(game_palette.palette)
    draw = ImageDraw.Draw(image)
    font = ImageFont.truetype("fonts/kenney/Kenney Future.ttf", 36)

    x_offset = 10
    y_offset = -24 + number
    draw.text((x_offset + 1, y_offset), ".", font=font, fill=black)
    draw.text((x_offset, y_offset - 1), ".", font=font, fill=gold)
    image.save(sys.argv[1] + "/pics/m_cursor" + str(number) + ".pcx")

# Generate main menu options
menu_options = {
    "m_main_game": "Game",
    "m_main_multiplayer": "Multiplayer",
    "m_main_options": "Options",
    "m_main_video": "Video",
    "m_main_quit": "Quit"
}

for key, option in menu_options.items():
    image = Image.new("P", (512, 32), transparent)
    image.putpalette(game_palette.palette)
    draw = ImageDraw.Draw(image)
    font = ImageFont.truetype("fonts/kenney/Kenney Future.ttf", 20)
    draw.text((0, 1), option, font=font, fill=black)
    draw.text((0, 0), option, font=font, fill=white)
    text_width = draw.textsize(option, font=font)[0];
    bounding_box = image.getbbox()
    image = image.crop((bounding_box[0], bounding_box[1], text_width, bounding_box[3]))
    image.save(sys.argv[1] + "/pics/" + key + ".pcx")

    # Draw selected
    image = Image.new("P", (512, 32), transparent)
    image.putpalette(game_palette.palette)
    draw = ImageDraw.Draw(image)
    font = ImageFont.truetype("fonts/kenney/Kenney Future.ttf", 20)
    draw.text((0, 1), option, font=font, fill=black)
    draw.text((0, 0), option, font=font, fill=gold)
    text_width = draw.textsize(option, font=font)[0];
    bounding_box = image.getbbox()
    image = image.crop((bounding_box[0], bounding_box[1], text_width, bounding_box[3]))
    image.save(sys.argv[1] + "/pics/" + key + "_sel.pcx")

# Generate main menu banners
title = "qengine"
image = Image.new("P", (38, 166), transparent)
image.putpalette(game_palette.palette)
draw = ImageDraw.Draw(image)
font = ImageFont.truetype("fonts/kenney/Kenney Future.ttf", 26)
text_height = 0

for letter in range(0, len(title)):
    text_width = draw.textsize(title[letter], font=font)[0];
    x_offset = max(text_width - 19, 0)

    if x_offset == 0:
        x_offset = 9

    draw.text((x_offset, text_height + 1), title[letter], font=font, fill=black)
    draw.text((x_offset, text_height), title[letter], font=font, fill=green)
    text_height += draw.textsize(title[letter], font=font)[1];

image.save(sys.argv[1] + "/pics/m_main_plaque.pcx")

# Generate banners
banners = {
    "pause": "Paused",
    "loading": "Loading",
    "m_banner_multiplayer": "Multiplayer",
    "m_banner_options": "Options",
    "m_banner_game": "Game",
    "m_banner_join_server": "Join Server",
    "m_banner_addressbook": "Addresses",
    "m_banner_video": "Video"
}

for key, banner in banners.items():
    image = Image.new("P", (512, 32), transparent)
    image.putpalette(game_palette.palette)
    draw = ImageDraw.Draw(image)
    font = ImageFont.truetype("fonts/kenney/Kenney Future.ttf", 20)
    draw.text((0, 1), banner, font=font, fill=black)
    draw.text((0, 0), banner, font=font, fill=white)
    text_width = draw.textsize(banner, font=font)[0];
    bounding_box = image.getbbox()
    image = image.crop((bounding_box[0], bounding_box[1], text_width, bounding_box[3]))
    image.save(sys.argv[1] + "/pics/" + key + ".pcx")

# Generate placeholder icons
icons = {
    "icon_health": "+",
    "icon_bodyarmor": "S1",
    "icon_combatarmor": "S2",
    "icon_jacketarmor": "S3",
    "icon_blaster": "W0",
    "icon_shotgun": "W1",
    "icon_sshotgun": "W2",
    "icon_machinegun": "W3",
    "icon_chaingun": "W4",
    "icon_grenades": "W5",
    "icon_glauncher": "W6",
    "icon_rlauncher": "W7",
    "icon_hyperblaster": "W8",
    "icon_railgun": "W9",
    "icon_bullets": "A1",
    "icon_shells": "A2",
    "icon_cells": "A3",
    "icon_rockets": "A4",
    "icon_slugs": "A5",
    "icon_bluekey": "K1",
    "icon_redkey": "K2",
}

for key, icon in icons.items():
    image = Image.new("P", (24, 24), white)
    image.putpalette(game_palette.palette)
    draw = ImageDraw.Draw(image)
    font = ImageFont.truetype("fonts/kenney/Kenney Future.ttf", 12)
    draw.text((2, 5), icon, font=font, fill=black)
    image.save(sys.argv[1] + "/pics/" + key + ".pcx")

# Generate crosshairs
icons = {
    "crosshair_1": "•",
    "crosshair_2": "·",
    "crosshair_3": "+"
}

for key, icon in icons.items():
    image = Image.new("P", (8, 8), transparent)
    image.putpalette(game_palette.palette)
    draw = ImageDraw.Draw(image)
    font = ImageFont.truetype("fonts/int10h/Px437_AmstradPC1512.ttf", 8)
    draw.text((0, 0), icon, font=font, fill=white)
    image.save(sys.argv[1] + "/pics/" + key + ".pcx")
