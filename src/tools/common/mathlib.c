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

// mathlib.c -- math primitives

#include "cmdlib.h"
#include "mathlib.h"

vec3_t vec3_origin = {0, 0, 0};

double VectorLength(vec3_t v)
{
  int i;
  double length;

  length = 0;
  for (i = 0; i < 3; i++)
    length += v[i] * v[i];
  length = sqrt(length); // FIXME

  return length;
}

qboolean VectorCompare(vec3_t v1, vec3_t v2)
{
  int i;

  for (i = 0; i < 3; i++)
    if (fabs(v1[i] - v2[i]) > EQUAL_EPSILON)
      return false;

  return true;
}

vec_t Q_rint(vec_t in)
{
  return floor(in + 0.5);
}

void VectorMA(vec3_t va, double scale, vec3_t vb, vec3_t vc)
{
  vc[0] = va[0] + scale * vb[0];
  vc[1] = va[1] + scale * vb[1];
  vc[2] = va[2] + scale * vb[2];
}

void CrossProduct(vec3_t v1, vec3_t v2, vec3_t cross)
{
  cross[0] = v1[1] * v2[2] - v1[2] * v2[1];
  cross[1] = v1[2] * v2[0] - v1[0] * v2[2];
  cross[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

vec_t _DotProduct(vec3_t v1, vec3_t v2)
{
  return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

void _VectorSubtract(vec3_t va, vec3_t vb, vec3_t out)
{
  out[0] = va[0] - vb[0];
  out[1] = va[1] - vb[1];
  out[2] = va[2] - vb[2];
}

void _VectorAdd(vec3_t va, vec3_t vb, vec3_t out)
{
  out[0] = va[0] + vb[0];
  out[1] = va[1] + vb[1];
  out[2] = va[2] + vb[2];
}

void _VectorCopy(vec3_t in, vec3_t out)
{
  out[0] = in[0];
  out[1] = in[1];
  out[2] = in[2];
}

void _VectorScale(vec3_t v, vec_t scale, vec3_t out)
{
  out[0] = v[0] * scale;
  out[1] = v[1] * scale;
  out[2] = v[2] * scale;
}

vec_t VectorNormalize(vec3_t in, vec3_t out)
{
  vec_t length, ilength;

  length = sqrt(in[0] * in[0] + in[1] * in[1] + in[2] * in[2]);
  if (length == 0) {
    VectorClear(out);
    return 0;
  }

  ilength = 1.0 / length;
  out[0] = in[0] * ilength;
  out[1] = in[1] * ilength;
  out[2] = in[2] * ilength;

  return length;
}

vec_t ColorNormalize(vec3_t in, vec3_t out)
{
  float max, scale;

  max = in[0];
  if (in[1] > max)
    max = in[1];
  if (in[2] > max)
    max = in[2];

  if (max == 0)
    return 0;

  scale = 1.0 / max;

  VectorScale(in, scale, out);

  return max;
}

void VectorInverse(vec3_t v)
{
  v[0] = -v[0];
  v[1] = -v[1];
  v[2] = -v[2];
}

void ClearBounds(vec3_t mins, vec3_t maxs)
{
  mins[0] = mins[1] = mins[2] = 99999;
  maxs[0] = maxs[1] = maxs[2] = -99999;
}

void AddPointToBounds(vec3_t v, vec3_t mins, vec3_t maxs)
{
  int i;
  vec_t val;

  for (i = 0; i < 3; i++) {
    val = v[i];
    if (val < mins[i])
      mins[i] = val;
    if (val > maxs[i])
      maxs[i] = val;
  }
}
