import os

from PIL import Image

BLACK = 0
WHITE = 15
GOLD = 56
GREEN = 208
OLIVE = 214
TRANSPARENT = 255


def load(assets):
    colormap = Image.open(os.path.join(assets, "pics/colormap.pcx"))

    return colormap.palette


def entries(assets):
    colormap = Image.open(os.path.join(assets, "pics/colormap.pcx"))
    values = colormap.getpalette()

    return [tuple(values[index * 3 : index * 3 + 3]) for index in range(256)]


def closest(colors, red, green, blue):
    best = BLACK
    distance = None

    for index, color in enumerate(colors):
        if index == TRANSPARENT:
            continue

        delta = (color[0] - red) ** 2 + (color[1] - green) ** 2 + (color[2] - blue) ** 2

        if distance is None or delta < distance:
            best = index
            distance = delta

    return best
