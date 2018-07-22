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

char mip_prefix[1024]; // directory to dump the textures in

qboolean colormap_issued;
byte colormap_palette[768];

/*
==============
RemapZero

Replaces all 0 bytes in an image with the closest palette entry.
This is because NT won't let us change index 0, so any palette
animation leaves those pixels untouched.
==============
*/
void RemapZero(byte *pixels, byte *palette, int width, int height)
{
  int i, c;
  int alt_zero;
  int value, best;

  alt_zero = 0;
  best = 9999999;
  for (i = 1; i < 255; i++) {
    value = palette[i * 3 + 0] + palette[i * 3 + 1] + palette[i * 3 + 2];
    if (value < best) {
      best = value;
      alt_zero = i;
    }
  }

  c = width * height;
  for (i = 0; i < c; i++)
    if (pixels[i] == 0)
      pixels[i] = alt_zero;
}

/*
==============
Cmd_Grab

$grab filename x y width height
==============
*/
void Cmd_Grab(void)
{
  int xl, yl, w, h, y;
  byte *cropped;
  char savename[1024];
  char dest[1024];

  GetToken(false);

  if (token[0] == '/' || token[0] == '\\')
    sprintf(savename, "%s%s.pcx", gamedir, token + 1);
  else
    sprintf(savename, "%spics/%s.pcx", gamedir, token);

  if (g_release) {
    if (token[0] == '/' || token[0] == '\\')
      sprintf(dest, "%s.pcx", token + 1);
    else
      sprintf(dest, "pics/%s.pcx", token);

    ReleaseFile(dest);
    return;
  }

  GetToken(false);
  xl = atoi(token);
  GetToken(false);
  yl = atoi(token);
  GetToken(false);
  w = atoi(token);
  GetToken(false);
  h = atoi(token);

  if (xl < 0 || yl < 0 || w < 0 || h < 0 || xl + w > byteimagewidth || yl + h > byteimageheight)
    Error("GrabPic: Bad size: %i, %i, %i, %i", xl, yl, w, h);

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
Cmd_Raw

$grab filename x y width height
==============
*/
void Cmd_Raw(void)
{
  int xl, yl, w, h, y;
  byte *cropped;
  char savename[1024];
  char dest[1024];

  GetToken(false);

  sprintf(savename, "%s%s.lmp", gamedir, token);

  if (g_release) {
    sprintf(dest, "%s.lmp", token);
    ReleaseFile(dest);
    return;
  }

  GetToken(false);
  xl = atoi(token);
  GetToken(false);
  yl = atoi(token);
  GetToken(false);
  w = atoi(token);
  GetToken(false);
  h = atoi(token);

  if (xl < 0 || yl < 0 || w < 0 || h < 0 || xl + w > byteimagewidth || yl + h > byteimageheight)
    Error("GrabPic: Bad size: %i, %i, %i, %i", xl, yl, w, h);

  // crop it to the proper size
  cropped = malloc(w * h);
  for (y = 0; y < h; y++) {
    memcpy(cropped + y * w, byteimage + (y + yl) * byteimagewidth + xl, w);
  }

  // save off the new image
  printf("saving %s\n", savename);
  CreatePath(savename);

  SaveFile(savename, cropped, w * h);

  free(cropped);
}

/*
=============================================================================

COLORMAP GRABBING

=============================================================================
*/

/*
===============
BestColor
===============
*/
byte BestColor(int r, int g, int b, int start, int stop)
{
  int i;
  int dr, dg, db;
  int bestdistortion, distortion;
  int bestcolor;
  byte *pal;

  //
  // let any color go to 0 as a last resort
  //
  bestdistortion = 256 * 256 * 4;
  bestcolor = 0;

  pal = colormap_palette + start * 3;
  for (i = start; i <= stop; i++) {
    dr = r - (int) pal[0];
    dg = g - (int) pal[1];
    db = b - (int) pal[2];
    pal += 3;
    distortion = dr * dr + dg * dg + db * db;
    if (distortion < bestdistortion) {
      if (!distortion)
        return i; // perfect match

      bestdistortion = distortion;
      bestcolor = i;
    }
  }

  return bestcolor;
}

/*
==============
Cmd_Colormap

$colormap filename

  the brightes colormap is first in the table (FIXME: reverse this now?)

  64 rows of 256 : lightmaps
  256 rows of 256 : translucency table
==============
*/
void Cmd_Colormap(void)
{
  int levels, brights;
  int l, c;
  float frac, red, green, blue;
  float range;
  byte *cropped, *lump_p;
  char savename[1024];
  char dest[1024];

  colormap_issued = true;
  if (!g_release)
    memcpy(colormap_palette, lbmpalette, 768);

  if (!TokenAvailable()) { // just setting colormap_issued
    return;
  }

  GetToken(false);
  sprintf(savename, "%spics/%s.pcx", gamedir, token);

  if (g_release) {
    sprintf(dest, "pics/%s.pcx", token);
    ReleaseFile(dest);
    return;
  }

  range = 2;
  levels = 64;
  brights = 1; // ignore 255 (transparent)

  cropped = malloc((levels + 256) * 256);
  lump_p = cropped;

  // shaded levels
  for (l = 0; l < levels; l++) {
    frac = range - range * (float) l / (levels - 1);
    for (c = 0; c < 256 - brights; c++) {
      red = lbmpalette[c * 3];
      green = lbmpalette[c * 3 + 1];
      blue = lbmpalette[c * 3 + 2];

      red = (int) (red * frac + 0.5);
      green = (int) (green * frac + 0.5);
      blue = (int) (blue * frac + 0.5);

      //
      // note: 254 instead of 255 because 255 is the transparent color, and we
      // don't want anything remapping to that
      // don't use color 0, because NT can't remap that (or 255)
      //
      *lump_p++ = BestColor(red, green, blue, 1, 254);
    }

    // fullbrights allways stay the same
    for (; c < 256; c++)
      *lump_p++ = c;
  }

  // 66% transparancy table
  for (l = 0; l < 255; l++) {
    for (c = 0; c < 255; c++) {
      red = lbmpalette[c * 3] * 0.33 + lbmpalette[l * 3] * 0.66;
      green = lbmpalette[c * 3 + 1] * 0.33 + lbmpalette[l * 3 + 1] * 0.66;
      blue = lbmpalette[c * 3 + 2] * 0.33 + lbmpalette[l * 3 + 2] * 0.66;

      *lump_p++ = BestColor(red, green, blue, 1, 254);
    }
    *lump_p++ = 255;
  }
  for (c = 0; c < 256; c++)
    *lump_p++ = 255;

  // save off the new image
  printf("saving %s\n", savename);
  CreatePath(savename);
  WritePCXfile(savename, cropped, 256, levels + 256, lbmpalette);

  free(cropped);
}

/*
=============================================================================

MIPTEX GRABBING

=============================================================================
*/

byte pixdata[256];

int d_red, d_green, d_blue;

byte palmap[32][32][32];
qboolean palmap_built;

/*
=============
FindColor
=============
*/
int FindColor(int r, int g, int b)
{
  int bestcolor;

  if (r > 255)
    r = 255;
  if (r < 0)
    r = 0;
  if (g > 255)
    g = 255;
  if (g < 0)
    g = 0;
  if (b > 255)
    b = 255;
  if (b < 0)
    b = 0;
#ifndef TABLECOLORS
  bestcolor = BestColor(r, g, b, 0, 254);
#else
  bestcolor = palmap[r >> 3][g >> 3][b >> 3];
#endif

  return bestcolor;
}

void BuildPalmap(void)
{
#ifdef TABLECOLORS
  int r, g, b;
  int bestcolor;

  if (palmap_built)
    return;
  palmap_built = true;

  for (r = 4; r < 256; r += 8) {
    for (g = 4; g < 256; g += 8) {
      for (b = 4; b < 256; b += 8) {
        bestcolor = BestColor(r, g, b, 1, 254);
        palmap[r >> 3][g >> 3][b >> 3] = bestcolor;
      }
    }
  }
#endif

  if (!colormap_issued)
    Error("You must issue a $colormap command first");
}

/*
=============
AveragePixels
=============
*/
byte AveragePixels(int count)
{
  int r, g, b;
  int i;
  int vis;
  int pix;
  int bestcolor;
  byte *pal;
  int fullbright;

  vis = 0;
  r = g = b = 0;
  fullbright = 0;
  for (i = 0; i < count; i++) {
    pix = pixdata[i];

    r += lbmpalette[pix * 3];
    g += lbmpalette[pix * 3 + 1];
    b += lbmpalette[pix * 3 + 2];
    vis++;
  }

  r /= vis;
  g /= vis;
  b /= vis;

  // error diffusion
  r += d_red;
  g += d_green;
  b += d_blue;

  //
  // find the best color
  //
  bestcolor = FindColor(r, g, b);

  // error diffusion
  pal = colormap_palette + bestcolor * 3;
  d_red = r - (int) pal[0];
  d_green = g - (int) pal[1];
  d_blue = b - (int) pal[2];

  return bestcolor;
}

typedef enum
{
  pt_contents,
  pt_flags,
  pt_animvalue,
  pt_flagvalue
} parmtype_t;

typedef struct
{
  char *name;
  int flags;
  parmtype_t type;
} mipparm_t;

mipparm_t mipparms[] = {
    // utility content attributes
    {"water", CONTENTS_WATER, pt_contents},
    {"slime", CONTENTS_SLIME, pt_contents},   // mildly damaging
    {"lava", CONTENTS_LAVA, pt_contents},     // very damaging
    {"window", CONTENTS_WINDOW, pt_contents}, // solid, but doesn't eat internal textures
    {"mist", CONTENTS_MIST, pt_contents},     // non-solid window
    {"origin", CONTENTS_ORIGIN, pt_contents}, // center of rotating brushes
    {"playerclip", CONTENTS_PLAYERCLIP, pt_contents},
    {"monsterclip", CONTENTS_MONSTERCLIP, pt_contents},

    // utility surface attributes
    {"hint", SURF_HINT, pt_flags},
    {"skip", SURF_SKIP, pt_flags},
    {"light", SURF_LIGHT, pt_flagvalue}, // value is the light quantity

    // texture chaining
    {"anim", 0, pt_animvalue}, // value is the next animation

    // server attributes
    {"slick", SURF_SLICK, pt_flags},

    // drawing attributes
    {"sky", SURF_SKY, pt_flags},
    {"warping", SURF_WARP, pt_flags},    // only valid with 64x64 textures
    {"trans33", SURF_TRANS33, pt_flags}, // translucent should allso set fullbright
    {"trans66", SURF_TRANS66, pt_flags},
    {"flowing", SURF_FLOWING, pt_flags}, // flow direction towards angle 0
    {"nodraw", SURF_NODRAW, pt_flags},   // for clip textures and trigger textures

    {NULL, 0, pt_contents}};

/*
==============
Cmd_Mip

$mip filename x y width height <OPTIONS>
must be multiples of sixteen
SURF_WINDOW
==============
*/
void Cmd_Mip(void)
{
  int x, y, xl, yl, xh, yh, w, h;
  byte *screen_p, *source;
  int linedelta;
  miptex_t *qtex;
  int miplevel, mipstep;
  int xx, yy, pix;
  int count;
  int flags, value, contents;
  mipparm_t *mp;
  char lumpname[64];
  byte *lump_p;
  char filename[1024];
  char animname[64];

  GetToken(false);
  strcpy(lumpname, token);

  GetToken(false);
  xl = atoi(token);
  GetToken(false);
  yl = atoi(token);
  GetToken(false);
  w = atoi(token);
  GetToken(false);
  h = atoi(token);

  if ((w & 15) || (h & 15))
    Error("line %i: miptex sizes must be multiples of 16", scriptline);

  flags = 0;
  contents = 0;
  value = 0;

  animname[0] = 0;

  // get optional flags and values
  while (TokenAvailable()) {
    GetToken(false);

    for (mp = mipparms; mp->name; mp++) {
      if (!strcmp(mp->name, token)) {
        switch (mp->type) {
        case pt_animvalue:
          GetToken(false); // specify the next animation frame
          strcpy(animname, token);
          break;
        case pt_flags:
          flags |= mp->flags;
          break;
        case pt_contents:
          contents |= mp->flags;
          break;
        case pt_flagvalue:
          flags |= mp->flags;
          GetToken(false); // specify the light value
          value = atoi(token);
          break;
        }
        break;
      }
    }
    if (!mp->name)
      Error("line %i: unknown parm %s", scriptline, token);
  }

  sprintf(filename, "%stextures/%s/%s.wal", gamedir, mip_prefix, lumpname);
  if (g_release)
    return; // textures are only released by $maps

  xh = xl + w;
  yh = yl + h;

  qtex = malloc(sizeof(miptex_t) + w * h * 2);
  memset(qtex, 0, sizeof(miptex_t));

  qtex->width = LittleLong(w);
  qtex->height = LittleLong(h);
  qtex->flags = LittleLong(flags);
  qtex->contents = LittleLong(contents);
  qtex->value = LittleLong(value);
  sprintf(qtex->name, "%s/%s", mip_prefix, lumpname);
  if (animname[0])
    sprintf(qtex->animname, "%s/%s", mip_prefix, animname);

  lump_p = (byte *) (&qtex->value + 1);

  screen_p = byteimage + yl * byteimagewidth + xl;
  linedelta = byteimagewidth - w;

  source = lump_p;
  qtex->offsets[0] = LittleLong(lump_p - (byte *) qtex);

  for (y = yl; y < yh; y++) {
    for (x = xl; x < xh; x++) {
      pix = *screen_p++;
      if (pix == 255)
        pix = 1; // should never happen
      *lump_p++ = pix;
    }
    screen_p += linedelta;
  }

  //
  // subsample for greater mip levels
  //
  d_red = d_green = d_blue = 0; // no distortion yet

  for (miplevel = 1; miplevel < 4; miplevel++) {
    qtex->offsets[miplevel] = LittleLong(lump_p - (byte *) qtex);

    mipstep = 1 << miplevel;
    for (y = 0; y < h; y += mipstep) {

      for (x = 0; x < w; x += mipstep) {
        count = 0;
        for (yy = 0; yy < mipstep; yy++)
          for (xx = 0; xx < mipstep; xx++) {
            pixdata[count] = source[(y + yy) * w + x + xx];
            count++;
          }
        *lump_p++ = AveragePixels(count);
      }
    }
  }

  //
  // dword align the size
  //
  while ((int) lump_p & 3)
    *lump_p++ = 0;

  //
  // write it out
  //
  printf("writing %s\n", filename);
  SaveFile(filename, (byte *) qtex, lump_p - (byte *) qtex);

  free(qtex);
}

/*
===============
Cmd_Mippal
===============
*/
void Cmd_Mippal(void)
{
  colormap_issued = true;
  if (g_release)
    return;

  memcpy(colormap_palette, lbmpalette, 768);

  BuildPalmap();
}

/*
===============
Cmd_Mipdir
===============
*/
void Cmd_Mipdir(void)
{
  char filename[1024];

  GetToken(false);
  strcpy(mip_prefix, token);
  // create the directory if needed
  sprintf(filename, "%stextures", gamedir, mip_prefix);
  Q_mkdir(filename);
  sprintf(filename, "%stextures/%s", gamedir, mip_prefix);
  Q_mkdir(filename);
}

/*
=============================================================================

ENVIRONMENT MAP GRABBING

Creates six pcx files from tga files without any palette edge seams
also copies the tga files for GL rendering.
=============================================================================
*/

// 3dstudio environment map suffixes
char *suf[6] = {"rt", "ft", "lf", "bk", "up", "dn"};

/*
=================
Cmd_Environment
=================
*/
void Cmd_Environment(void)
{
  char name[1024];
  int i, x, y;
  byte image[256 * 256];
  byte *tga;

  GetToken(false);

  if (g_release) {
    for (i = 0; i < 6; i++) {
      sprintf(name, "env/%s%s.pcx", token, suf[i]);
      ReleaseFile(name);
      sprintf(name, "env/%s%s.tga", token, suf[i]);
      ReleaseFile(name);
    }
    return;
  }
  // get the palette
  BuildPalmap();

  sprintf(name, "%senv/", gamedir);
  CreatePath(name);

  // convert the images
  for (i = 0; i < 6; i++) {
    sprintf(name, "%senv/%s%s.tga", gamedir, token, suf[i]);
    printf("loading %s...\n", name);
    LoadTGA(name, &tga, NULL, NULL);

    for (y = 0; y < 256; y++) {
      for (x = 0; x < 256; x++) {
        image[y * 256 + x] =
            FindColor(tga[(y * 256 + x) * 4 + 0], tga[(y * 256 + x) * 4 + 1], tga[(y * 256 + x) * 4 + 2]);
      }
    }
    free(tga);
    sprintf(name, "%senv/%s%s.pcx", gamedir, token, suf[i]);
    if (FileTime(name) != -1)
      printf("%s already exists, not overwriting.\n", name);
    else
      WritePCXfile(name, image, 256, 256, colormap_palette);
  }
}
