# assetgen
This module contains various base asset generation tools written in Python.

## fontgen
Allows you to generate a `conchars.pcx` asset, used by the engine to display text on the menus and the console. Very easy to use:

    $ ./fontgen.py /your/game/assets/folder

The tool will automatically fetch the game palette from your `colormap.pcx` and generate the `conchars.pcx`.

## uigen
Allows you to generate various UI elements, such as health and ammo numbers, menu options and banners.

    $ ./uigen.py /your/game/assets/folder

The tool will automatically fetch the game palette from your `colormap.pcx` and generate the asset files.
