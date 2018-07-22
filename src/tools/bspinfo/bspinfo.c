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

#include "cmdlib.h"
#include "mathlib.h"
#include "bspfile.h"

void main(int argc, char **argv)
{
  int i;
  char source[1024];
  int size;
  FILE *f;

  if (argc == 1)
    Error("usage: bspinfo bspfile [bspfiles]");

  for (i = 1; i < argc; i++) {
    printf("---------------------\n");
    strcpy(source, argv[i]);
    DefaultExtension(source, ".bsp");
    f = fopen(source, "rb");
    if (f) {
      size = Q_filelength(f);
      fclose(f);
    } else
      size = 0;
    printf("%s: %i\n", source, size);

    LoadBSPFile(source);
    PrintBSPFileSizes();
    printf("---------------------\n");
  }
}
