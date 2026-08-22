#!/usr/bin/env python3
import os
import sys

from PIL import Image, ImageDraw, ImageFont

import palette

ROOT = os.path.dirname(os.path.abspath(__file__))

TITLE_FONT = os.path.join(ROOT, "fonts/kenney/Kenney Future.ttf")
SMALL_FONT = os.path.join(ROOT, "fonts/int10h/Px437_AmstradPC1512.ttf")

assets = None
colors = None
entries = None


def new_image(width, height, background=palette.TRANSPARENT):
    image = Image.new("P", (width, height), background)
    image.putpalette(colors)

    return image


def save(image, name):
    image.save(os.path.join(assets, "pics", name + ".pcx"))
    print(name + ".pcx")


def text_size(draw, text, font):
    box = draw.textbbox((0, 0), text, font=font)

    return box[2], box[3]


def shade(red, green, blue):
    return palette.closest(entries, red, green, blue)


def draw_character(character, group, color, shadow_color, character_name=None):
    if not character_name:
        character_name = character

    image = new_image(16, 24)
    draw = ImageDraw.Draw(image)
    font = ImageFont.truetype(TITLE_FONT, 24)
    draw.text((1, -3), character, font=font, fill=shadow_color)
    draw.text((0, -4), character, font=font, fill=color)
    save(image, group + "_" + character_name)


def draw_label(text, name, color, size=20):
    image = new_image(512, 32)
    draw = ImageDraw.Draw(image)
    font = ImageFont.truetype(TITLE_FONT, size)
    draw.text((0, 1), text, font=font, fill=palette.BLACK)
    draw.text((0, 0), text, font=font, fill=color)
    text_width = text_size(draw, text, font)[0]
    bounding_box = image.getbbox()
    image = image.crop((bounding_box[0], bounding_box[1], text_width, bounding_box[3]))
    save(image, name)


def generate_numbers():
    for number in range(0, 10):
        draw_character(str(number), "num", palette.WHITE, palette.BLACK)

    for number in range(0, 10):
        draw_character(str(number), "anum", palette.OLIVE, palette.BLACK)

    draw_character("-", "num", palette.WHITE, palette.BLACK, "minus")
    draw_character("-", "anum", palette.OLIVE, palette.BLACK, "minus")


def generate_cursor():
    for number in range(0, 16):
        image = new_image(22, 29)
        draw = ImageDraw.Draw(image)
        font = ImageFont.truetype(TITLE_FONT, 36)

        x_offset = 10
        y_offset = -24 + number
        draw.text((x_offset + 1, y_offset), ".", font=font, fill=palette.BLACK)
        draw.text((x_offset, y_offset - 1), ".", font=font, fill=palette.GOLD)
        save(image, "m_cursor" + str(number))


def generate_menu_options():
    menu_options = {
        "m_main_game": "Game",
        "m_main_multiplayer": "Multiplayer",
        "m_main_options": "Options",
        "m_main_video": "Video",
        "m_main_quit": "Quit",
    }

    for key, option in menu_options.items():
        draw_label(option, key, palette.WHITE)
        draw_label(option, key + "_sel", palette.GOLD)


def generate_plaque():
    title = "qengine"
    image = new_image(38, 166)
    draw = ImageDraw.Draw(image)
    font = ImageFont.truetype(TITLE_FONT, 26)
    text_height = 0

    for letter in range(0, len(title)):
        width, height = text_size(draw, title[letter], font)
        x_offset = max(width - 19, 0)

        if x_offset == 0:
            x_offset = 9

        draw.text(
            (x_offset, text_height + 1), title[letter], font=font, fill=palette.BLACK
        )
        draw.text((x_offset, text_height), title[letter], font=font, fill=palette.GREEN)
        text_height += height

    save(image, "m_main_plaque")


def generate_banners():
    banners = {
        "pause": "Paused",
        "loading": "Loading",
        "m_banner_multiplayer": "Multiplayer",
        "m_banner_options": "Options",
        "m_banner_game": "Game",
        "m_banner_join_server": "Join Server",
        "m_banner_addressbook": "Addresses",
        "m_banner_video": "Video",
    }

    for key, banner in banners.items():
        draw_label(banner, key, palette.WHITE)


def generate_icons():
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
        image = new_image(24, 24, palette.WHITE)
        draw = ImageDraw.Draw(image)
        font = ImageFont.truetype(TITLE_FONT, 12)
        draw.text((2, 5), icon, font=font, fill=palette.BLACK)
        save(image, key)


def generate_crosshairs():
    crosshairs = {
        "crosshair_1": "•",
        "crosshair_2": "·",
        "crosshair_3": "+",
    }

    for key, crosshair in crosshairs.items():
        image = new_image(8, 8)
        draw = ImageDraw.Draw(image)
        font = ImageFont.truetype(SMALL_FONT, 8)
        draw.text((0, 0), crosshair, font=font, fill=palette.WHITE)
        save(image, key)


def generate_backtile():
    size = 64
    base = shade(24, 24, 28)
    panel = shade(40, 40, 46)
    highlight = shade(60, 60, 68)

    image = new_image(size, size, base)
    draw = ImageDraw.Draw(image)

    for y in range(0, size, 32):
        for x in range(0, size, 32):
            draw.rectangle((x + 2, y + 2, x + 29, y + 29), fill=panel)
            draw.line((x + 2, y + 2, x + 29, y + 2), fill=highlight)
            draw.line((x + 2, y + 2, x + 2, y + 29), fill=highlight)

    save(image, "backtile")


def generate_inventory():
    width = 256
    height = 192
    background = shade(16, 16, 20)
    border = shade(72, 72, 80)
    accent = shade(120, 120, 130)

    image = new_image(width, height, background)
    draw = ImageDraw.Draw(image)
    draw.rectangle((0, 0, width - 1, height - 1), outline=border)
    draw.rectangle((2, 2, width - 3, height - 3), outline=accent)
    draw.line((2, 14, width - 3, 14), fill=border)

    save(image, "inventory")


def generate_tags():
    tags = {
        "tag1": palette.GOLD,
        "tag2": palette.GREEN,
    }

    for name, color in tags.items():
        image = new_image(16, 16)
        draw = ImageDraw.Draw(image)
        draw.ellipse((1, 1, 14, 14), fill=palette.BLACK)
        draw.ellipse((0, 0, 13, 13), fill=color)
        draw.ellipse((4, 4, 9, 9), fill=palette.BLACK)
        save(image, name)


def generate_conback():
    width = 320
    height = 240
    image = new_image(width, height, shade(10, 10, 12))
    draw = ImageDraw.Draw(image)

    grain = [shade(14, 14, 17), shade(18, 18, 22), shade(10, 10, 12)]
    noise = 0x2F6E2B1

    for y in range(height):
        for x in range(0, width, 2):
            noise = (noise * 199001225245 + 12345) & 0x7FFFFFFF

            if noise >> 28 == 0:
                image.putpixel((x, y), grain[(x + y) % len(grain)])

    for y in range(0, height, 3):
        draw.line((0, y, width - 1, y), fill=shade(6, 6, 8))

    font = ImageFont.truetype(TITLE_FONT, 42)
    title = "qengine"
    text_width, text_height = text_size(draw, title, font)
    x = (width - text_width) // 2
    y = (height - text_height) // 2
    draw.text((x + 2, y + 2), title, font=font, fill=shade(4, 4, 5))
    draw.text((x, y), title, font=font, fill=shade(58, 58, 66))

    draw.line((0, height - 2, width - 1, height - 2), fill=shade(70, 70, 80))
    draw.line((0, height - 1, width - 1, height - 1), fill=shade(24, 24, 28))

    save(image, "conback")


def generate(target):
    global assets, colors, entries

    assets = target
    colors = palette.load(target)
    entries = palette.entries(target)

    generate_numbers()
    generate_cursor()
    generate_menu_options()
    generate_plaque()
    generate_banners()
    generate_icons()
    generate_crosshairs()
    generate_conback()
    generate_backtile()
    generate_inventory()
    generate_tags()


if __name__ == "__main__":
    if len(sys.argv) < 2:
        sys.exit("usage: uigen.py <assets directory>")

    generate(sys.argv[1])
