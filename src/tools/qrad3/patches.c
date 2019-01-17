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

#include "qrad.h"

vec3_t texture_reflectivity[MAX_MAP_TEXINFO];

/*
===================================================================

  TEXTURE LIGHT VALUES

===================================================================
*/

/*
======================
CalcTextureReflectivity
======================
*/
void CalcTextureReflectivity(void)
{
  int i;
  int j, k, texels;
  int color[3];
  int texel;
  byte *palette;
  char path[1024];
  float r, scale;
  miptex_t *mt;

  sprintf(path, "%s../pics/colormap.pcx", gamedir);

  // get the game palette
  Load256Image(path, NULL, &palette, NULL, NULL);

  // allways set index 0 even if no textures
  texture_reflectivity[0][0] = 0.5;
  texture_reflectivity[0][1] = 0.5;
  texture_reflectivity[0][2] = 0.5;

  for (i = 0; i < numtexinfo; i++) {
    // see if an earlier texinfo allready got the value
    for (j = 0; j < i; j++) {
      if (!strcmp(texinfo[i].texture, texinfo[j].texture)) {
        VectorCopy(texture_reflectivity[j], texture_reflectivity[i]);
        break;
      }
    }
    if (j != i)
      continue;

    // load the wal file
    sprintf(path, "%s../textures/%s.wal", gamedir, texinfo[i].texture);
    if (TryLoadFile(path, (void **) &mt) == -1) {
      printf("Couldn't load %s\n", path);
      texture_reflectivity[i][0] = 0.5;
      texture_reflectivity[i][1] = 0.5;
      texture_reflectivity[i][2] = 0.5;
      continue;
    }
    texels = LittleLong(mt->width) * LittleLong(mt->height);
    color[0] = color[1] = color[2] = 0;

    for (j = 0; j < texels; j++) {
      texel = ((byte *) mt)[LittleLong(mt->offsets[0]) + j];
      for (k = 0; k < 3; k++)
        color[k] += palette[texel * 3 + k];
    }

    for (j = 0; j < 3; j++) {
      r = color[j] / texels / 255.0;
      texture_reflectivity[i][j] = r;
    }
    // scale the reflectivity up, because the textures are
    // so dim
    scale = ColorNormalize(texture_reflectivity[i], texture_reflectivity[i]);
    if (scale < 0.5) {
      scale *= 2;
      VectorScale(texture_reflectivity[i], scale, texture_reflectivity[i]);
    }
#if 0
        texture_reflectivity[i][0] = 0.5;
        texture_reflectivity[i][1] = 0.5;
        texture_reflectivity[i][2] = 0.5;
#endif
  }
}

/*
=======================================================================

MAKE FACES

=======================================================================
*/

/*
=============
WindingFromFace
=============
*/
winding_t *WindingFromFace(dface_t *f)
{
  int i;
  int se;
  dvertex_t *dv;
  int v;
  winding_t *w;

  w = AllocWinding(f->numedges);
  w->numpoints = f->numedges;

  for (i = 0; i < f->numedges; i++) {
    se = dsurfedges[f->firstedge + i];
    if (se < 0)
      v = dedges[-se].v[1];
    else
      v = dedges[se].v[0];

    dv = &dvertexes[v];
    VectorCopy(dv->point, w->p[i]);
  }

  RemoveColinearPoints(w);

  return w;
}

/*
=============
BaseLightForFace
=============
*/
void BaseLightForFace(dface_t *f, vec3_t color)
{
  texinfo_t *tx;

  //
  // check for light emited by texture
  //
  tx = &texinfo[f->texinfo];
  if (!(tx->flags & SURF_LIGHT) || tx->value == 0) {
    VectorClear(color);
    return;
  }

  VectorScale(texture_reflectivity[f->texinfo], tx->value, color);
}

qboolean IsSky(dface_t *f)
{
  texinfo_t *tx;

  tx = &texinfo[f->texinfo];
  if (tx->flags & SURF_SKY)
    return true;
  return false;
}

/*
=============
MakePatchForFace
=============
*/
float totalarea;

void MakePatchForFace(int fn, winding_t *w)
{
  dface_t *f;
  float area;
  patch_t *patch;
  dplane_t *pl;
  int i;
  vec3_t color;
  dleaf_t *leaf;

  f = &dfaces[fn];

  area = WindingArea(w);
  totalarea += area;

  patch = &patches[num_patches];
  if (num_patches == MAX_PATCHES)
    Error("num_patches == MAX_PATCHES");
  patch->next = face_patches[fn];
  face_patches[fn] = patch;

  patch->winding = w;

  if (f->side)
    patch->plane = &backplanes[f->planenum];
  else
    patch->plane = &dplanes[f->planenum];
  if (face_offset[fn][0] || face_offset[fn][1] || face_offset[fn][2]) { // origin offset faces must create new planes
    if (numplanes + fakeplanes >= MAX_MAP_PLANES)
      Error("numplanes + fakeplanes >= MAX_MAP_PLANES");
    pl = &dplanes[numplanes + fakeplanes];
    fakeplanes++;

    *pl = *(patch->plane);
    pl->dist += DotProduct(face_offset[fn], pl->normal);
    patch->plane = pl;
  }

  WindingCenter(w, patch->origin);
  VectorAdd(patch->origin, patch->plane->normal, patch->origin);
  leaf = PointInLeaf(patch->origin);
  patch->cluster = leaf->cluster;
  if (patch->cluster == -1)
    qprintf("patch->cluster == -1\n");

  patch->area = area;
  if (patch->area <= 1)
    patch->area = 1;
  patch->sky = IsSky(f);

  VectorCopy(texture_reflectivity[f->texinfo], patch->reflectivity);

  // non-bmodel patches can emit light
  if (fn < dmodels[0].numfaces) {
    BaseLightForFace(f, patch->baselight);

    ColorNormalize(patch->reflectivity, color);

    for (i = 0; i < 3; i++)
      patch->baselight[i] *= color[i];

    VectorCopy(patch->baselight, patch->totallight);
  }
  num_patches++;
}

entity_t *EntityForModel(int modnum)
{
  int i;
  char *s;
  char name[16];

  sprintf(name, "*%i", modnum);
  // search the entities for one using modnum
  for (i = 0; i < num_entities; i++) {
    s = ValueForKey(&entities[i], "model");
    if (!strcmp(s, name))
      return &entities[i];
  }

  return &entities[0];
}

/*
=============
MakePatches
=============
*/
void MakePatches(void)
{
  int i, j, k;
  dface_t *f;
  int fn;
  winding_t *w;
  dmodel_t *mod;
  vec3_t origin;
  entity_t *ent;

  qprintf("%i faces\n", numfaces);

  for (i = 0; i < nummodels; i++) {
    mod = &dmodels[i];
    ent = EntityForModel(i);
    // bmodels with origin brushes need to be offset into their
    // in-use position
    GetVectorForKey(ent, "origin", origin);
    // VectorCopy (vec3_origin, origin);

    for (j = 0; j < mod->numfaces; j++) {
      fn = mod->firstface + j;
      face_entity[fn] = ent;
      VectorCopy(origin, face_offset[fn]);
      f = &dfaces[fn];
      w = WindingFromFace(f);
      for (k = 0; k < w->numpoints; k++) {
        VectorAdd(w->p[k], origin, w->p[k]);
      }
      MakePatchForFace(fn, w);
    }
  }

  qprintf("%i sqaure feet\n", (int) (totalarea / 64));
}

/*
=======================================================================

SUBDIVIDE

=======================================================================
*/

void FinishSplit(patch_t *patch, patch_t *newp)
{
  dleaf_t *leaf;

  VectorCopy(patch->baselight, newp->baselight);
  VectorCopy(patch->totallight, newp->totallight);
  VectorCopy(patch->reflectivity, newp->reflectivity);
  newp->plane = patch->plane;
  newp->sky = patch->sky;

  patch->area = WindingArea(patch->winding);
  newp->area = WindingArea(newp->winding);

  if (patch->area <= 1)
    patch->area = 1;
  if (newp->area <= 1)
    newp->area = 1;

  WindingCenter(patch->winding, patch->origin);
  VectorAdd(patch->origin, patch->plane->normal, patch->origin);
  leaf = PointInLeaf(patch->origin);
  patch->cluster = leaf->cluster;
  if (patch->cluster == -1)
    qprintf("patch->cluster == -1\n");

  WindingCenter(newp->winding, newp->origin);
  VectorAdd(newp->origin, newp->plane->normal, newp->origin);
  leaf = PointInLeaf(newp->origin);
  newp->cluster = leaf->cluster;
  if (newp->cluster == -1)
    qprintf("patch->cluster == -1\n");
}

/*
=============
SubdividePatch

Chops the patch only if its local bounds exceed the max size
=============
*/
void SubdividePatch(patch_t *patch)
{
  winding_t *w, *o1, *o2;
  vec3_t mins, maxs, total;
  vec3_t split;
  vec_t dist;
  int i, j;
  vec_t v;
  patch_t *newp;

  w = patch->winding;
  mins[0] = mins[1] = mins[2] = 99999;
  maxs[0] = maxs[1] = maxs[2] = -99999;
  for (i = 0; i < w->numpoints; i++) {
    for (j = 0; j < 3; j++) {
      v = w->p[i][j];
      if (v < mins[j])
        mins[j] = v;
      if (v > maxs[j])
        maxs[j] = v;
    }
  }
  VectorSubtract(maxs, mins, total);
  for (i = 0; i < 3; i++)
    if (total[i] > (subdiv + 1))
      break;
  if (i == 3) {
    // no splitting needed
    return;
  }

  //
  // split the winding
  //
  VectorCopy(vec3_origin, split);
  split[i] = 1;
  dist = (mins[i] + maxs[i]) * 0.5;
  ClipWindingEpsilon(w, split, dist, ON_EPSILON, &o1, &o2);

  //
  // create a new patch
  //
  if (num_patches == MAX_PATCHES)
    Error("MAX_PATCHES");
  newp = &patches[num_patches];
  num_patches++;

  newp->next = patch->next;
  patch->next = newp;

  patch->winding = o1;
  newp->winding = o2;

  FinishSplit(patch, newp);

  SubdividePatch(patch);
  SubdividePatch(newp);
}

/*
=============
DicePatch

Chops the patch by a global grid
=============
*/
void DicePatch(patch_t *patch)
{
  winding_t *w, *o1, *o2;
  vec3_t mins, maxs;
  vec3_t split;
  vec_t dist;
  int i;
  patch_t *newp;

  w = patch->winding;
  WindingBounds(w, mins, maxs);
  for (i = 0; i < 3; i++)
    if (floor((mins[i] + 1) / subdiv) < floor((maxs[i] - 1) / subdiv))
      break;
  if (i == 3) {
    // no splitting needed
    return;
  }

  //
  // split the winding
  //
  VectorCopy(vec3_origin, split);
  split[i] = 1;
  dist = subdiv * (1 + floor((mins[i] + 1) / subdiv));
  ClipWindingEpsilon(w, split, dist, ON_EPSILON, &o1, &o2);

  //
  // create a new patch
  //
  if (num_patches == MAX_PATCHES)
    Error("MAX_PATCHES");
  newp = &patches[num_patches];
  num_patches++;

  newp->next = patch->next;
  patch->next = newp;

  patch->winding = o1;
  newp->winding = o2;

  FinishSplit(patch, newp);

  DicePatch(patch);
  DicePatch(newp);
}

/*
=============
SubdividePatches
=============
*/
void SubdividePatches(void)
{
  int i, num;

  if (subdiv < 1)
    return;

  num = num_patches; // because the list will grow
  for (i = 0; i < num; i++) {
    //		SubdividePatch (&patches[i]);
    DicePatch(&patches[i]);
  }
  qprintf("%i patches after subdivision\n", num_patches);
}

//=====================================================================
