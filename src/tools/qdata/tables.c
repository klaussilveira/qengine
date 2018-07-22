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

/*
=============================================================================

ALPHALIGHT GENERATION

Find alphamap values that best match modulated lightmap values

This isn't used anymore, but I'm keeping it around...
=============================================================================
*/

unsigned short alphamap[32 * 32 * 32];
unsigned char inverse16to8table[65536];

/*
static int FindNearestColor( unsigned int color )
{
        int i;
        int closest_so_far = 0;
        float closest_distance_so_far = 100000000;
        float d;
        float r[2], g[2], b[2];

        // incoming color is assumed to be in 0xRRGGBB format
        r[0] = ( color & 31 ) << 3;
        g[0] = ( ( color >> 5 ) & 63 ) << 2;
        b[0] = ( ( color >> 11 ) & 31 ) << 3;

        for ( i = 0; i < 256; i++ )
        {
                r[1] = ( d_8to24table[i] >> 0 ) & 0xFF;
                g[1] = ( d_8to24table[i] >> 8 ) & 0xFF;
                b[1] = ( d_8to24table[i] >> 16 ) & 0xFF;

                d = ( r[1] - r[0] ) * ( r[1] - r[0] ) +
                        ( g[1] - g[0] ) * ( g[1] - g[0] ) +
                        ( b[1] - b[0] ) * ( b[1] - b[0] );

                if ( d < closest_distance_so_far )
                {
                        closest_distance_so_far = d;
                        closest_so_far = i;
                }
        }

        return closest_so_far;
}
*/

extern byte BestColor(int, int, int, int, int);

void Inverse16_BuildTable(void)
{
  int i;

  /*
  ** create the 16-to-8 table
  */
  for (i = 0; i < 65536; i++) {
    int r = i & 31;
    int g = (i >> 5) & 63;
    int b = (i >> 11) & 31;

    r <<= 3;
    g <<= 2;
    b <<= 3;

    inverse16to8table[i] = BestColor(r, g, b, 0, 255);
  }
}

void Alphalight_Thread(int i)
{
  int j;
  float r, g, b;
  float mr, mg, mb, ma;
  float distortion, bestdistortion;
  float v;

  r = (i >> 10) * (1.0 / 16);
  g = ((i >> 5) & 31) * (1.0 / 16);
  b = (i & 31) * (1.0 / 16);

  bestdistortion = 999999;
  for (j = 0; j < 16 * 16 * 16 * 16; j++) {
    mr = (j >> 12) * (1.0 / 16);
    mg = ((j >> 8) & 15) * (1.0 / 16);
    mb = ((j >> 4) & 15) * (1.0 / 16);
    ma = (j & 15) * (1.0 / 16);

    v = r * 0.5 - (mr * ma + 0.5 * (1.0 - ma));
    distortion = v * v;
    v = g * 0.5 - (mg * ma + 0.5 * (1.0 - ma));
    distortion += v * v;
    v = b * 0.5 - (mb * ma + 0.5 * (1.0 - ma));
    distortion += v * v;

    distortion *= 1.0 + ma * 4;

    if (distortion < bestdistortion) {
      bestdistortion = distortion;
      alphamap[i] = j;
    }
  }
}

void Cmd_Alphalight(void)
{
  char savename[1024];

  GetToken(false);

  if (g_release) {
    ReleaseFile(token);
    return;
  }

  sprintf(savename, "%s%s", gamedir, token);
  printf("Building alphalight table...\n");

  RunThreadsOnIndividual(32 * 32 * 32, true, Alphalight_Thread);

  SaveFile(savename, (byte *) alphamap, sizeof(alphamap));
}

void Cmd_Inverse16Table(void)
{
  char savename[1024];

  if (g_release) {
    sprintf(savename, "pics/16to8.dat");
    ReleaseFile(savename);
    return;
  }

  sprintf(savename, "%spics/16to8.dat", gamedir);
  printf("Building inverse 16-to-8 table...\n");

  Inverse16_BuildTable();

  SaveFile(savename, (byte *) inverse16to8table, sizeof(inverse16to8table));
}
