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

typedef unsigned char UBYTE;
// conflicts with windows typedef short			WORD;
typedef unsigned short UWORD;
typedef long LONG;

typedef enum
{
  ms_none,
  ms_mask,
  ms_transcolor,
  ms_lasso
} mask_t;

typedef enum
{
  cm_none,
  cm_rle1
} compress_t;

typedef struct
{
  UWORD w, h;
  short x, y;
  UBYTE nPlanes;
  UBYTE masking;
  UBYTE compression;
  UBYTE pad1;
  UWORD transparentColor;
  UBYTE xAspect, yAspect;
  short pageWidth, pageHeight;
} bmhd_t;

extern bmhd_t bmhd; // will be in native byte order

#define FORMID ('F' + ('O' << 8) + ((int) 'R' << 16) + ((int) 'M' << 24))
#define ILBMID ('I' + ('L' << 8) + ((int) 'B' << 16) + ((int) 'M' << 24))
#define PBMID ('P' + ('B' << 8) + ((int) 'M' << 16) + ((int) ' ' << 24))
#define BMHDID ('B' + ('M' << 8) + ((int) 'H' << 16) + ((int) 'D' << 24))
#define BODYID ('B' + ('O' << 8) + ((int) 'D' << 16) + ((int) 'Y' << 24))
#define CMAPID ('C' + ('M' << 8) + ((int) 'A' << 16) + ((int) 'P' << 24))

bmhd_t bmhd;

void LoadLBM(char *filename, byte **picture, byte **palette);

void WriteLBMfile(char *filename, byte *data, int width, int height, byte *palette);

void LoadPCX(char *filename, byte **picture, byte **palette, int *width, int *height);

void WritePCXfile(char *filename, byte *data, int width, int height, byte *palette);

// loads / saves either lbm or pcx, depending on extension
void Load256Image(char *name, byte **pixels, byte **palette, int *width, int *height);

void Save256Image(char *name, byte *pixels, byte *palette, int width, int height);

void LoadTGA(char *filename, byte **pixels, int *width, int *height);
