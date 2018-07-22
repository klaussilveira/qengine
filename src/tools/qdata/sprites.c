/*
===========================================================================
Copyright (C) 1997-2006 Id Software, Inc.

This file is part of Quake 2 Tools source code.

Quake 2 Tools source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake 2 Tools source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake 2 Tools source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "qdata.h"

#define MAX_SPRFRAMES MAX_MD2SKINS

dsprite_t sprite;
dsprframe_t frames[MAX_SPRFRAMES];

byte *byteimage, *lbmpalette;
int byteimagewidth, byteimageheight;

char spritename[1024];

void FinishSprite(void);

void Cmd_Spritename(void);

/*
==============
FinishSprite
==============
*/
void FinishSprite(void)
{
  FILE *spriteouthandle;
  int i, curframe;
  dsprite_t spritetemp;
  char savename[1024];

  if (sprite.numframes == 0)
    return;

  if (!strlen(spritename))
    Error("Didn't name sprite file");

  sprintf(savename, "%s%s.sp2", gamedir, spritename);

  if (g_release) {
    char name[1024];

    sprintf(name, "%s.sp2", spritename);
    ReleaseFile(name);
    spritename[0] = 0; // clear for a new sprite
    sprite.numframes = 0;
    return;
  }

  printf("saving in %s\n", savename);
  CreatePath(savename);
  spriteouthandle = SafeOpenWrite(savename);

  //
  // write out the sprite header
  //
  spritetemp.ident = LittleLong(IDSPRITEHEADER);
  spritetemp.version = LittleLong(SPRITE_VERSION);
  spritetemp.numframes = LittleLong(sprite.numframes);

  SafeWrite(spriteouthandle, &spritetemp, 12);

  //
  // write out the frames
  //
  curframe = 0;

  for (i = 0; i < sprite.numframes; i++) {
    frames[i].width = LittleLong(frames[i].width);
    frames[i].height = LittleLong(frames[i].height);
    frames[i].origin_x = LittleLong(frames[i].origin_x);
    frames[i].origin_y = LittleLong(frames[i].origin_y);
  }
  SafeWrite(spriteouthandle, frames, sizeof(frames[0]) * sprite.numframes);

  fclose(spriteouthandle);

  spritename[0] = 0; // clear for a new sprite
  sprite.numframes = 0;
}

/*
===============
Cmd_Load
===============
*/
void Cmd_Load(void)
{
  char *name;

  GetToken(false);

  if (g_release)
    return;

  name = ExpandPathAndArchive(token);

  // load the image
  printf("loading %s\n", name);
  Load256Image(name, &byteimage, &lbmpalette, &byteimagewidth, &byteimageheight);
  RemapZero(byteimage, lbmpalette, byteimagewidth, byteimageheight);
}

/*
===============
Cmd_SpriteFrame
===============
*/
void Cmd_SpriteFrame(void)
{
  int y, xl, yl, xh, yh, w, h;
  dsprframe_t *pframe;
  int ox, oy;
  byte *cropped;
  char savename[1024];

  GetToken(false);
  xl = atoi(token);
  GetToken(false);
  yl = atoi(token);
  GetToken(false);
  w = atoi(token);
  GetToken(false);
  h = atoi(token);

  // origin offset is optional
  if (TokenAvailable()) {
    GetToken(false);
    ox = atoi(token);
    GetToken(false);
    oy = atoi(token);
  } else {
    ox = w / 2;
    oy = h / 2;
  }

  if ((xl & 0x07) || (yl & 0x07) || (w & 0x07) || (h & 0x07))
    Error("Sprite dimensions not multiples of 8\n");

  if ((w > 256) || (h > 256))
    Error("Sprite has a dimension longer than 256");

  xh = xl + w;
  yh = yl + h;

  if (sprite.numframes >= MAX_SPRFRAMES)
    Error("Too many frames; increase MAX_SPRFRAMES\n");

  pframe = &frames[sprite.numframes];
  pframe->width = w;
  pframe->height = h;
  pframe->origin_x = ox;
  pframe->origin_y = oy;
  sprintf(pframe->name, "%s_%i.pcx", spritename, sprite.numframes);
  sprintf(savename, "%s%s_%i.pcx", gamedir, spritename, sprite.numframes);
  sprite.numframes++;

  if (g_release) {
    ReleaseFile(pframe->name);
    return;
  }

  // crop it to the proper size
  cropped = malloc(w * h);
  for (y = 0; y < h; y++) {
    memcpy(cropped + y * w, byteimage + (y + yl) * byteimagewidth + xl, w);
  }

  // save off the new image
  printf("saving %s\n", savename);
  CreatePath(savename);
  WritePCXfile(savename, cropped, w, h, lbmpalette);

  free(cropped);
}

/*
==============
Cmd_SpriteName
==============
*/
void Cmd_SpriteName(void)
{
  if (sprite.numframes)
    FinishSprite();

  GetToken(false);
  strcpy(spritename, token);
  memset(&sprite, 0, sizeof(sprite));
  memset(&frames, 0, sizeof(frames));
}
