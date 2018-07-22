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

/*

NOTES
-----

every surface must be divided into at least two patches each axis

*/

patch_t *face_patches[MAX_MAP_FACES];
entity_t *face_entity[MAX_MAP_FACES];
patch_t patches[MAX_PATCHES];
unsigned num_patches;

vec3_t radiosity[MAX_PATCHES];    // light leaving a patch
vec3_t illumination[MAX_PATCHES]; // light arriving at a patch

vec3_t face_offset[MAX_MAP_FACES]; // for rotating bmodels
dplane_t backplanes[MAX_MAP_PLANES];

char inbase[32], outbase[32];

int fakeplanes; // created planes for origin offset

int numbounce = 8;
qboolean extrasamples;

float subdiv = 64;
qboolean dumppatches;

void BuildLightmaps(void);

int TestLine(vec3_t start, vec3_t stop);

int junk;

float ambient = 0;
float maxlight = 196;

float lightscale = 1.0;

qboolean glview;

qboolean nopvs;

char source[1024];

float direct_scale = 0.4;
float entity_scale = 1.0;

/*
===================================================================

MISC

===================================================================
*/

/*
=============
MakeBackplanes
=============
*/
void MakeBackplanes(void)
{
  int i;

  for (i = 0; i < numplanes; i++) {
    backplanes[i].dist = -dplanes[i].dist;
    VectorSubtract(vec3_origin, dplanes[i].normal, backplanes[i].normal);
  }
}

int leafparents[MAX_MAP_LEAFS];
int nodeparents[MAX_MAP_NODES];

/*
=============
MakeParents
=============
*/
void MakeParents(int nodenum, int parent)
{
  int i, j;
  dnode_t *node;

  nodeparents[nodenum] = parent;
  node = &dnodes[nodenum];

  for (i = 0; i < 2; i++) {
    j = node->children[i];
    if (j < 0)
      leafparents[-j - 1] = nodenum;
    else
      MakeParents(j, nodenum);
  }
}

/*
===================================================================

TRANSFER SCALES

===================================================================
*/

int PointInLeafnum(vec3_t point)
{
  int nodenum;
  vec_t dist;
  dnode_t *node;
  dplane_t *plane;

  nodenum = 0;
  while (nodenum >= 0) {
    node = &dnodes[nodenum];
    plane = &dplanes[node->planenum];
    dist = DotProduct(point, plane->normal) - plane->dist;
    if (dist > 0)
      nodenum = node->children[0];
    else
      nodenum = node->children[1];
  }

  return -nodenum - 1;
}

dleaf_t *PointInLeaf(vec3_t point)
{
  int num;

  num = PointInLeafnum(point);
  return &dleafs[num];
}

qboolean PvsForOrigin(vec3_t org, byte *pvs)
{
  dleaf_t *leaf;

  if (!visdatasize) {
    memset(pvs, 255, (numleafs + 7) / 8);
    return true;
  }

  leaf = PointInLeaf(org);
  if (leaf->cluster == -1)
    return false; // in solid leaf

  DecompressVis(dvisdata + dvis->bitofs[leaf->cluster][DVIS_PVS], pvs);
  return true;
}

/*
=============
MakeTransfers

=============
*/
int total_transfer;

void MakeTransfers(int i)
{
  int j;
  vec3_t delta;
  vec_t dist, scale;
  float trans;
  int itrans;
  patch_t *patch, *patch2;
  float total;
  dplane_t plane;
  vec3_t origin;
  float transfers[MAX_PATCHES], *all_transfers;
  int s;
  int itotal;
  byte pvs[(MAX_MAP_LEAFS + 7) / 8];
  int cluster;

  patch = patches + i;
  total = 0;

  VectorCopy(patch->origin, origin);
  plane = *patch->plane;

  if (!PvsForOrigin(patch->origin, pvs))
    return;

  // find out which patch2s will collect light
  // from patch

  all_transfers = transfers;
  patch->numtransfers = 0;
  for (j = 0, patch2 = patches; j < num_patches; j++, patch2++) {
    transfers[j] = 0;

    if (j == i)
      continue;

    // check pvs bit
    if (!nopvs) {
      cluster = patch2->cluster;
      if (cluster == -1)
        continue;
      if (!(pvs[cluster >> 3] & (1 << (cluster & 7))))
        continue; // not in pvs
    }

    // calculate vector
    VectorSubtract(patch2->origin, origin, delta);
    dist = VectorNormalize(delta, delta);
    if (!dist)
      continue; // should never happen

    // reletive angles
    scale = DotProduct(delta, plane.normal);
    scale *= -DotProduct(delta, patch2->plane->normal);
    if (scale <= 0)
      continue;

    // check exact tramsfer
    if (TestLine_r(0, patch->origin, patch2->origin))
      continue;

    trans = scale * patch2->area / (dist * dist);

    if (trans < 0)
      trans = 0; // rounding errors...

    transfers[j] = trans;
    if (trans > 0) {
      total += trans;
      patch->numtransfers++;
    }
  }

  // copy the transfers out and normalize
  // total should be somewhere near PI if everything went right
  // because partial occlusion isn't accounted for, and nearby
  // patches have underestimated form factors, it will usually
  // be higher than PI
  if (patch->numtransfers) {
    transfer_t *t;

    if (patch->numtransfers < 0 || patch->numtransfers > MAX_PATCHES)
      Error("Weird numtransfers");
    s = patch->numtransfers * sizeof(transfer_t);
    patch->transfers = malloc(s);
    if (!patch->transfers)
      Error("Memory allocation failure");

    //
    // normalize all transfers so all of the light
    // is transfered to the surroundings
    //
    t = patch->transfers;
    itotal = 0;
    for (j = 0; j < num_patches; j++) {
      if (transfers[j] <= 0)
        continue;
      itrans = transfers[j] * 0x10000 / total;
      itotal += itrans;
      t->transfer = itrans;
      t->patch = j;
      t++;
    }
  }

  // don't bother locking around this.  not that important.
  total_transfer += patch->numtransfers;
}

/*
=============
FreeTransfers
=============
*/
void FreeTransfers(void)
{
  int i;

  for (i = 0; i < num_patches; i++) {
    free(patches[i].transfers);
    patches[i].transfers = NULL;
  }
}

//===================================================================

/*
=============
WriteWorld
=============
*/
void WriteWorld(char *name)
{
  int i, j;
  FILE *out;
  patch_t *patch;
  winding_t *w;

  out = fopen(name, "w");
  if (!out)
    Error("Couldn't open %s", name);

  for (j = 0, patch = patches; j < num_patches; j++, patch++) {
    w = patch->winding;
    fprintf(out, "%i\n", w->numpoints);
    for (i = 0; i < w->numpoints; i++) {
      fprintf(out, "%5.2f %5.2f %5.2f %5.3f %5.3f %5.3f\n", w->p[i][0], w->p[i][1], w->p[i][2], patch->totallight[0],
              patch->totallight[1], patch->totallight[2]);
    }
    fprintf(out, "\n");
  }

  fclose(out);
}

/*
=============
WriteGlView
=============
*/
void WriteGlView(void)
{
  char name[1024];
  FILE *f;
  int i, j;
  patch_t *p;
  winding_t *w;

  strcpy(name, source);
  StripExtension(name);
  strcat(name, ".glr");

  f = fopen(name, "w");
  if (!f)
    Error("Couldn't open %s", f);

  for (j = 0; j < num_patches; j++) {
    p = &patches[j];
    w = p->winding;
    fprintf(f, "%i\n", w->numpoints);
    for (i = 0; i < w->numpoints; i++) {
      fprintf(f, "%5.2f %5.2f %5.2f %5.3f %5.3f %5.3f\n", w->p[i][0], w->p[i][1], w->p[i][2], p->totallight[0] / 128,
              p->totallight[1] / 128, p->totallight[2] / 128);
    }
    fprintf(f, "\n");
  }

  fclose(f);
}

//==============================================================

/*
=============
CollectLight
=============
*/
float CollectLight(void)
{
  int i, j;
  patch_t *patch;
  vec_t total;

  total = 0;

  for (i = 0, patch = patches; i < num_patches; i++, patch++) {
    // skys never collect light, it is just dropped
    if (patch->sky) {
      VectorClear(radiosity[i]);
      VectorClear(illumination[i]);
      continue;
    }

    for (j = 0; j < 3; j++) {
      patch->totallight[j] += illumination[i][j] / patch->area;
      radiosity[i][j] = illumination[i][j] * patch->reflectivity[j];
    }

    total += radiosity[i][0] + radiosity[i][1] + radiosity[i][2];
    VectorClear(illumination[i]);
  }

  return total;
}

/*
=============
ShootLight

Send light out to other patches
  Run multi-threaded
=============
*/
void ShootLight(int patchnum)
{
  int k, l;
  transfer_t *trans;
  int num;
  patch_t *patch;
  vec3_t send;

  // this is the amount of light we are distributing
  // prescale it so that multiplying by the 16 bit
  // transfer values gives a proper output value
  for (k = 0; k < 3; k++)
    send[k] = radiosity[patchnum][k] / 0x10000;
  patch = &patches[patchnum];

  trans = patch->transfers;
  num = patch->numtransfers;

  for (k = 0; k < num; k++, trans++) {
    for (l = 0; l < 3; l++)
      illumination[trans->patch][l] += send[l] * trans->transfer;
  }
}

/*
=============
BounceLight
=============
*/
void BounceLight(void)
{
  int i, j;
  float added;
  char name[64];
  patch_t *p;

  for (i = 0; i < num_patches; i++) {
    p = &patches[i];
    for (j = 0; j < 3; j++) {
      //			p->totallight[j] = p->samplelight[j];
      radiosity[i][j] = p->samplelight[j] * p->reflectivity[j] * p->area;
    }
  }

  for (i = 0; i < numbounce; i++) {
    RunThreadsOnIndividual(num_patches, false, ShootLight);
    added = CollectLight();

    qprintf("bounce:%i added:%f\n", i, added);
    if (dumppatches && (i == 0 || i == numbounce - 1)) {
      sprintf(name, "bounce%i.txt", i);
      WriteWorld(name);
    }
  }
}

//==============================================================

void CheckPatches(void)
{
  int i;
  patch_t *patch;

  for (i = 0; i < num_patches; i++) {
    patch = &patches[i];
    if (patch->totallight[0] < 0 || patch->totallight[1] < 0 || patch->totallight[2] < 0)
      Error("negative patch totallight\n");
  }
}

/*
=============
RadWorld
=============
*/
void RadWorld(void)
{
  if (numnodes == 0 || numfaces == 0)
    Error("Empty map");
  MakeBackplanes();
  MakeParents(0, -1);
  MakeTnodes(&dmodels[0]);

  // turn each face into a single patch
  MakePatches();

  // subdivide patches to a maximum dimension
  SubdividePatches();

  // create directlights out of patches and lights
  CreateDirectLights();

  // build initial facelights
  RunThreadsOnIndividual(numfaces, true, BuildFacelights);

  if (numbounce > 0) {
    // build transfer lists
    RunThreadsOnIndividual(num_patches, true, MakeTransfers);
    qprintf("transfer lists: %5.1f megs\n", (float) total_transfer * sizeof(transfer_t) / (1024 * 1024));

    // spread light around
    BounceLight();

    FreeTransfers();

    CheckPatches();
  }

  if (glview)
    WriteGlView();

  // blend bounced light into direct light and save
  PairEdges();
  LinkPlaneFaces();

  lightdatasize = 0;
  RunThreadsOnIndividual(numfaces, true, FinalLightFace);
}

/*
========
main

light modelfile
========
*/
int main(int argc, char **argv)
{
  int i;
  double start, end;
  char name[1024];

  printf("----- Radiosity ----\n");

  verbose = false;

  for (i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-dump"))
      dumppatches = true;
    else if (!strcmp(argv[i], "-bounce")) {
      numbounce = atoi(argv[i + 1]);
      i++;
    } else if (!strcmp(argv[i], "-v")) {
      verbose = true;
    } else if (!strcmp(argv[i], "-extra")) {
      extrasamples = true;
      printf("extrasamples = true\n");
    } else if (!strcmp(argv[i], "-threads")) {
      numthreads = atoi(argv[i + 1]);
      i++;
    } else if (!strcmp(argv[i], "-chop")) {
      subdiv = atoi(argv[i + 1]);
      i++;
    } else if (!strcmp(argv[i], "-scale")) {
      lightscale = atof(argv[i + 1]);
      i++;
    } else if (!strcmp(argv[i], "-direct")) {
      direct_scale *= atof(argv[i + 1]);
      printf("direct light scaling at %f\n", direct_scale);
      i++;
    } else if (!strcmp(argv[i], "-entity")) {
      entity_scale *= atof(argv[i + 1]);
      printf("entity light scaling at %f\n", entity_scale);
      i++;
    } else if (!strcmp(argv[i], "-glview")) {
      glview = true;
      printf("glview = true\n");
    } else if (!strcmp(argv[i], "-nopvs")) {
      nopvs = true;
      printf("nopvs = true\n");
    } else if (!strcmp(argv[i], "-ambient")) {
      ambient = atof(argv[i + 1]) * 128;
      i++;
    } else if (!strcmp(argv[i], "-maxlight")) {
      maxlight = atof(argv[i + 1]) * 128;
      i++;
    } else if (!strcmp(argv[i], "-tmpin"))
      strcpy(inbase, "/tmp");
    else if (!strcmp(argv[i], "-tmpout"))
      strcpy(outbase, "/tmp");
    else
      break;
  }

  ThreadSetDefault();

  if (maxlight > 255)
    maxlight = 255;

  if (i != argc - 1)
    Error("usage: qrad [-v] [-chop num] [-scale num] [-ambient num] [-maxlight "
          "num] [-threads num] bspfile");

  start = I_FloatTime();

  SetQdirFromPath(argv[i]);
  strcpy(source, ExpandArg(argv[i]));
  StripExtension(source);
  DefaultExtension(source, ".bsp");

  //	ReadLightFile ();

  sprintf(name, "%s%s", inbase, source);
  printf("reading %s\n", name);
  LoadBSPFile(name);
  ParseEntities();
  CalcTextureReflectivity();

  if (!visdatasize) {
    printf("No vis information, direct lighting only.\n");
    numbounce = 0;
    ambient = 0.1;
  }

  RadWorld();

  sprintf(name, "%s%s", outbase, source);
  printf("writing %s\n", name);
  WriteBSPFile(name);

  end = I_FloatTime();
  printf("%5.0f seconds elapsed\n", end - start);

  return 0;
}
