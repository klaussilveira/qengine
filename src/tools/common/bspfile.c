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
#include "scriplib.h"

void GetLeafNums(void);

//=============================================================================

int nummodels;
dmodel_t dmodels[MAX_MAP_MODELS];

int visdatasize;
byte dvisdata[MAX_MAP_VISIBILITY];
dvis_t *dvis = (dvis_t *) dvisdata;

int lightdatasize;
byte dlightdata[MAX_MAP_LIGHTING];

int entdatasize;
char dentdata[MAX_MAP_ENTSTRING];

int numleafs;
dleaf_t dleafs[MAX_MAP_LEAFS];

int numplanes;
dplane_t dplanes[MAX_MAP_PLANES];

int numvertexes;
dvertex_t dvertexes[MAX_MAP_VERTS];

int numnodes;
dnode_t dnodes[MAX_MAP_NODES];

int numtexinfo;
texinfo_t texinfo[MAX_MAP_TEXINFO];

int numfaces;
dface_t dfaces[MAX_MAP_FACES];

int numedges;
dedge_t dedges[MAX_MAP_EDGES];

int numleaffaces;
unsigned short dleaffaces[MAX_MAP_LEAFFACES];

int numleafbrushes;
unsigned short dleafbrushes[MAX_MAP_LEAFBRUSHES];

int numsurfedges;
int dsurfedges[MAX_MAP_SURFEDGES];

int numbrushes;
dbrush_t dbrushes[MAX_MAP_BRUSHES];

int numbrushsides;
dbrushside_t dbrushsides[MAX_MAP_BRUSHSIDES];

int numareas;
darea_t dareas[MAX_MAP_AREAS];

int numareaportals;
dareaportal_t dareaportals[MAX_MAP_AREAPORTALS];

byte dpop[256];

/*
===============
CompressVis

===============
*/
int CompressVis(byte *vis, byte *dest)
{
  int j;
  int rep;
  int visrow;
  byte *dest_p;

  dest_p = dest;
  //	visrow = (r_numvisleafs + 7)>>3;
  visrow = (dvis->numclusters + 7) >> 3;

  for (j = 0; j < visrow; j++) {
    *dest_p++ = vis[j];
    if (vis[j])
      continue;

    rep = 1;
    for (j++; j < visrow; j++)
      if (vis[j] || rep == 255)
        break;
      else
        rep++;
    *dest_p++ = rep;
    j--;
  }

  return dest_p - dest;
}

/*
===================
DecompressVis
===================
*/
void DecompressVis(byte *in, byte *decompressed)
{
  int c;
  byte *out;
  int row;

  //	row = (r_numvisleafs+7)>>3;
  row = (dvis->numclusters + 7) >> 3;
  out = decompressed;

  do {
    if (*in) {
      *out++ = *in++;
      continue;
    }

    c = in[1];
    if (!c)
      Error("DecompressVis: 0 repeat");
    in += 2;
    while (c) {
      *out++ = 0;
      c--;
    }
  } while (out - decompressed < row);
}

//=============================================================================

/*
=============
SwapBSPFile

Byte swaps all data in a bsp file.
=============
*/
void SwapBSPFile(qboolean todisk)
{
  int i, j;
  dmodel_t *d;

  // models
  for (i = 0; i < nummodels; i++) {
    d = &dmodels[i];

    d->firstface = LittleLong(d->firstface);
    d->numfaces = LittleLong(d->numfaces);
    d->headnode = LittleLong(d->headnode);

    for (j = 0; j < 3; j++) {
      d->mins[j] = LittleFloat(d->mins[j]);
      d->maxs[j] = LittleFloat(d->maxs[j]);
      d->origin[j] = LittleFloat(d->origin[j]);
    }
  }

  //
  // vertexes
  //
  for (i = 0; i < numvertexes; i++) {
    for (j = 0; j < 3; j++)
      dvertexes[i].point[j] = LittleFloat(dvertexes[i].point[j]);
  }

  //
  // planes
  //
  for (i = 0; i < numplanes; i++) {
    for (j = 0; j < 3; j++)
      dplanes[i].normal[j] = LittleFloat(dplanes[i].normal[j]);
    dplanes[i].dist = LittleFloat(dplanes[i].dist);
    dplanes[i].type = LittleLong(dplanes[i].type);
  }

  //
  // texinfos
  //
  for (i = 0; i < numtexinfo; i++) {
    for (j = 0; j < 8; j++)
      texinfo[i].vecs[0][j] = LittleFloat(texinfo[i].vecs[0][j]);
    texinfo[i].flags = LittleLong(texinfo[i].flags);
    texinfo[i].value = LittleLong(texinfo[i].value);
    texinfo[i].nexttexinfo = LittleLong(texinfo[i].nexttexinfo);
  }

  //
  // faces
  //
  for (i = 0; i < numfaces; i++) {
    dfaces[i].texinfo = LittleShort(dfaces[i].texinfo);
    dfaces[i].planenum = LittleShort(dfaces[i].planenum);
    dfaces[i].side = LittleShort(dfaces[i].side);
    dfaces[i].lightofs = LittleLong(dfaces[i].lightofs);
    dfaces[i].firstedge = LittleLong(dfaces[i].firstedge);
    dfaces[i].numedges = LittleShort(dfaces[i].numedges);
  }

  //
  // nodes
  //
  for (i = 0; i < numnodes; i++) {
    dnodes[i].planenum = LittleLong(dnodes[i].planenum);
    for (j = 0; j < 3; j++) {
      dnodes[i].mins[j] = LittleShort(dnodes[i].mins[j]);
      dnodes[i].maxs[j] = LittleShort(dnodes[i].maxs[j]);
    }
    dnodes[i].children[0] = LittleLong(dnodes[i].children[0]);
    dnodes[i].children[1] = LittleLong(dnodes[i].children[1]);
    dnodes[i].firstface = LittleShort(dnodes[i].firstface);
    dnodes[i].numfaces = LittleShort(dnodes[i].numfaces);
  }

  //
  // leafs
  //
  for (i = 0; i < numleafs; i++) {
    dleafs[i].contents = LittleLong(dleafs[i].contents);
    dleafs[i].cluster = LittleShort(dleafs[i].cluster);
    dleafs[i].area = LittleShort(dleafs[i].area);
    for (j = 0; j < 3; j++) {
      dleafs[i].mins[j] = LittleShort(dleafs[i].mins[j]);
      dleafs[i].maxs[j] = LittleShort(dleafs[i].maxs[j]);
    }

    dleafs[i].firstleafface = LittleShort(dleafs[i].firstleafface);
    dleafs[i].numleaffaces = LittleShort(dleafs[i].numleaffaces);
    dleafs[i].firstleafbrush = LittleShort(dleafs[i].firstleafbrush);
    dleafs[i].numleafbrushes = LittleShort(dleafs[i].numleafbrushes);
  }

  //
  // leaffaces
  //
  for (i = 0; i < numleaffaces; i++)
    dleaffaces[i] = LittleShort(dleaffaces[i]);

  //
  // leafbrushes
  //
  for (i = 0; i < numleafbrushes; i++)
    dleafbrushes[i] = LittleShort(dleafbrushes[i]);

  //
  // surfedges
  //
  for (i = 0; i < numsurfedges; i++)
    dsurfedges[i] = LittleLong(dsurfedges[i]);

  //
  // edges
  //
  for (i = 0; i < numedges; i++) {
    dedges[i].v[0] = LittleShort(dedges[i].v[0]);
    dedges[i].v[1] = LittleShort(dedges[i].v[1]);
  }

  //
  // brushes
  //
  for (i = 0; i < numbrushes; i++) {
    dbrushes[i].firstside = LittleLong(dbrushes[i].firstside);
    dbrushes[i].numsides = LittleLong(dbrushes[i].numsides);
    dbrushes[i].contents = LittleLong(dbrushes[i].contents);
  }

  //
  // areas
  //
  for (i = 0; i < numareas; i++) {
    dareas[i].numareaportals = LittleLong(dareas[i].numareaportals);
    dareas[i].firstareaportal = LittleLong(dareas[i].firstareaportal);
  }

  //
  // areasportals
  //
  for (i = 0; i < numareaportals; i++) {
    dareaportals[i].portalnum = LittleLong(dareaportals[i].portalnum);
    dareaportals[i].otherarea = LittleLong(dareaportals[i].otherarea);
  }

  //
  // brushsides
  //
  for (i = 0; i < numbrushsides; i++) {
    dbrushsides[i].planenum = LittleShort(dbrushsides[i].planenum);
    dbrushsides[i].texinfo = LittleShort(dbrushsides[i].texinfo);
  }

  //
  // visibility
  //
  if (todisk)
    j = dvis->numclusters;
  else
    j = LittleLong(dvis->numclusters);
  dvis->numclusters = LittleLong(dvis->numclusters);
  for (i = 0; i < j; i++) {
    dvis->bitofs[i][0] = LittleLong(dvis->bitofs[i][0]);
    dvis->bitofs[i][1] = LittleLong(dvis->bitofs[i][1]);
  }
}

dheader_t *header;

int CopyLump(int lump, void *dest, int size)
{
  int length, ofs;

  length = header->lumps[lump].filelen;
  ofs = header->lumps[lump].fileofs;

  if (length % size)
    Error("LoadBSPFile: odd lump size");

  memcpy(dest, (byte *) header + ofs, length);

  return length / size;
}

/*
=============
LoadBSPFile
=============
*/
void LoadBSPFile(char *filename)
{
  int i;

  //
  // load the file header
  //
  LoadFile(filename, (void **) &header);

  // swap the header
  for (i = 0; i < sizeof(dheader_t) / 4; i++)
    ((int *) header)[i] = LittleLong(((int *) header)[i]);

  if (header->ident != IDBSPHEADER)
    Error("%s is not a IBSP file", filename);
  if (header->version != BSPVERSION)
    Error("%s is version %i, not %i", filename, header->version, BSPVERSION);

  nummodels = CopyLump(LUMP_MODELS, dmodels, sizeof(dmodel_t));
  numvertexes = CopyLump(LUMP_VERTEXES, dvertexes, sizeof(dvertex_t));
  numplanes = CopyLump(LUMP_PLANES, dplanes, sizeof(dplane_t));
  numleafs = CopyLump(LUMP_LEAFS, dleafs, sizeof(dleaf_t));
  numnodes = CopyLump(LUMP_NODES, dnodes, sizeof(dnode_t));
  numtexinfo = CopyLump(LUMP_TEXINFO, texinfo, sizeof(texinfo_t));
  numfaces = CopyLump(LUMP_FACES, dfaces, sizeof(dface_t));
  numleaffaces = CopyLump(LUMP_LEAFFACES, dleaffaces, sizeof(dleaffaces[0]));
  numleafbrushes = CopyLump(LUMP_LEAFBRUSHES, dleafbrushes, sizeof(dleafbrushes[0]));
  numsurfedges = CopyLump(LUMP_SURFEDGES, dsurfedges, sizeof(dsurfedges[0]));
  numedges = CopyLump(LUMP_EDGES, dedges, sizeof(dedge_t));
  numbrushes = CopyLump(LUMP_BRUSHES, dbrushes, sizeof(dbrush_t));
  numbrushsides = CopyLump(LUMP_BRUSHSIDES, dbrushsides, sizeof(dbrushside_t));
  numareas = CopyLump(LUMP_AREAS, dareas, sizeof(darea_t));
  numareaportals = CopyLump(LUMP_AREAPORTALS, dareaportals, sizeof(dareaportal_t));

  visdatasize = CopyLump(LUMP_VISIBILITY, dvisdata, 1);
  lightdatasize = CopyLump(LUMP_LIGHTING, dlightdata, 1);
  entdatasize = CopyLump(LUMP_ENTITIES, dentdata, 1);

  CopyLump(LUMP_POP, dpop, 1);

  free(header); // everything has been copied out

  //
  // swap everything
  //
  SwapBSPFile(false);
}

/*
=============
LoadBSPFileTexinfo

Only loads the texinfo lump, so qdata can scan for textures
=============
*/
void LoadBSPFileTexinfo(char *filename)
{
  int i;
  FILE *f;
  int length, ofs;

  header = malloc(sizeof(dheader_t));

  f = fopen(filename, "rb");
  fread(header, sizeof(dheader_t), 1, f);

  // swap the header
  for (i = 0; i < sizeof(dheader_t) / 4; i++)
    ((int *) header)[i] = LittleLong(((int *) header)[i]);

  if (header->ident != IDBSPHEADER)
    Error("%s is not a IBSP file", filename);
  if (header->version != BSPVERSION)
    Error("%s is version %i, not %i", filename, header->version, BSPVERSION);

  length = header->lumps[LUMP_TEXINFO].filelen;
  ofs = header->lumps[LUMP_TEXINFO].fileofs;

  fseek(f, ofs, SEEK_SET);
  fread(texinfo, length, 1, f);
  fclose(f);

  numtexinfo = length / sizeof(texinfo_t);

  free(header); // everything has been copied out

  SwapBSPFile(false);
}

//============================================================================

FILE *wadfile;
dheader_t outheader;

void AddLump(int lumpnum, void *data, int len)
{
  lump_t *lump;

  lump = &header->lumps[lumpnum];

  lump->fileofs = LittleLong(ftell(wadfile));
  lump->filelen = LittleLong(len);
  SafeWrite(wadfile, data, (len + 3) & ~3);
}

/*
=============
WriteBSPFile

Swaps the bsp file in place, so it should not be referenced again
=============
*/
void WriteBSPFile(char *filename)
{
  header = &outheader;
  memset(header, 0, sizeof(dheader_t));

  SwapBSPFile(true);

  header->ident = LittleLong(IDBSPHEADER);
  header->version = LittleLong(BSPVERSION);

  wadfile = SafeOpenWrite(filename);
  SafeWrite(wadfile, header, sizeof(dheader_t)); // overwritten later

  AddLump(LUMP_PLANES, dplanes, numplanes * sizeof(dplane_t));
  AddLump(LUMP_LEAFS, dleafs, numleafs * sizeof(dleaf_t));
  AddLump(LUMP_VERTEXES, dvertexes, numvertexes * sizeof(dvertex_t));
  AddLump(LUMP_NODES, dnodes, numnodes * sizeof(dnode_t));
  AddLump(LUMP_TEXINFO, texinfo, numtexinfo * sizeof(texinfo_t));
  AddLump(LUMP_FACES, dfaces, numfaces * sizeof(dface_t));
  AddLump(LUMP_BRUSHES, dbrushes, numbrushes * sizeof(dbrush_t));
  AddLump(LUMP_BRUSHSIDES, dbrushsides, numbrushsides * sizeof(dbrushside_t));
  AddLump(LUMP_LEAFFACES, dleaffaces, numleaffaces * sizeof(dleaffaces[0]));
  AddLump(LUMP_LEAFBRUSHES, dleafbrushes, numleafbrushes * sizeof(dleafbrushes[0]));
  AddLump(LUMP_SURFEDGES, dsurfedges, numsurfedges * sizeof(dsurfedges[0]));
  AddLump(LUMP_EDGES, dedges, numedges * sizeof(dedge_t));
  AddLump(LUMP_MODELS, dmodels, nummodels * sizeof(dmodel_t));
  AddLump(LUMP_AREAS, dareas, numareas * sizeof(darea_t));
  AddLump(LUMP_AREAPORTALS, dareaportals, numareaportals * sizeof(dareaportal_t));

  AddLump(LUMP_LIGHTING, dlightdata, lightdatasize);
  AddLump(LUMP_VISIBILITY, dvisdata, visdatasize);
  AddLump(LUMP_ENTITIES, dentdata, entdatasize);
  AddLump(LUMP_POP, dpop, sizeof(dpop));

  fseek(wadfile, 0, SEEK_SET);
  SafeWrite(wadfile, header, sizeof(dheader_t));
  fclose(wadfile);
}

//============================================================================

/*
=============
PrintBSPFileSizes

Dumps info about current file
=============
*/
void PrintBSPFileSizes(void)
{
  if (!num_entities)
    ParseEntities();

  printf("%5i models       %7i\n", nummodels, (int) (nummodels * sizeof(dmodel_t)));
  printf("%5i brushes      %7i\n", numbrushes, (int) (numbrushes * sizeof(dbrush_t)));
  printf("%5i brushsides   %7i\n", numbrushsides, (int) (numbrushsides * sizeof(dbrushside_t)));
  printf("%5i planes       %7i\n", numplanes, (int) (numplanes * sizeof(dplane_t)));
  printf("%5i texinfo      %7i\n", numtexinfo, (int) (numtexinfo * sizeof(texinfo_t)));
  printf("%5i entdata      %7i\n", num_entities, entdatasize);

  printf("\n");

  printf("%5i vertexes     %7i\n", numvertexes, (int) (numvertexes * sizeof(dvertex_t)));
  printf("%5i nodes        %7i\n", numnodes, (int) (numnodes * sizeof(dnode_t)));
  printf("%5i faces        %7i\n", numfaces, (int) (numfaces * sizeof(dface_t)));
  printf("%5i leafs        %7i\n", numleafs, (int) (numleafs * sizeof(dleaf_t)));
  printf("%5i leaffaces    %7i\n", numleaffaces, (int) (numleaffaces * sizeof(dleaffaces[0])));
  printf("%5i leafbrushes  %7i\n", numleafbrushes, (int) (numleafbrushes * sizeof(dleafbrushes[0])));
  printf("%5i surfedges    %7i\n", numsurfedges, (int) (numsurfedges * sizeof(dsurfedges[0])));
  printf("%5i edges        %7i\n", numedges, (int) (numedges * sizeof(dedge_t)));
  printf("      lightdata    %7i\n", lightdatasize);
  printf("      visdata      %7i\n", visdatasize);
}

//============================================

int num_entities;
entity_t entities[MAX_MAP_ENTITIES];

void StripTrailing(char *e)
{
  char *s;

  s = e + strlen(e) - 1;
  while (s >= e && *s <= 32) {
    *s = 0;
    s--;
  }
}

/*
=================
ParseEpair
=================
*/
epair_t *ParseEpair(void)
{
  epair_t *e;

  e = malloc(sizeof(epair_t));
  memset(e, 0, sizeof(epair_t));

  if (strlen(token) >= MAX_KEY - 1)
    Error("ParseEpar: token too long");
  e->key = copystring(token);
  GetToken(false);
  if (strlen(token) >= MAX_VALUE - 1)
    Error("ParseEpar: token too long");
  e->value = copystring(token);

  // strip trailing spaces
  StripTrailing(e->key);
  StripTrailing(e->value);

  return e;
}

/*
================
ParseEntity
================
*/
qboolean ParseEntity(void)
{
  epair_t *e;
  entity_t *mapent;

  if (!GetToken(true))
    return false;

  if (strcmp(token, "{"))
    Error("ParseEntity: { not found");

  if (num_entities == MAX_MAP_ENTITIES)
    Error("num_entities == MAX_MAP_ENTITIES");

  mapent = &entities[num_entities];
  num_entities++;

  do {
    if (!GetToken(true))
      Error("ParseEntity: EOF without closing brace");
    if (!strcmp(token, "}"))
      break;
    e = ParseEpair();
    e->next = mapent->epairs;
    mapent->epairs = e;
  } while (1);

  return true;
}

/*
================
ParseEntities

Parses the dentdata string into entities
================
*/
void ParseEntities(void)
{
  num_entities = 0;
  ParseFromMemory(dentdata, entdatasize);

  while (ParseEntity()) {
  }
}

/*
================
UnparseEntities

Generates the dentdata string from all the entities
================
*/
void UnparseEntities(void)
{
  char *buf, *end;
  epair_t *ep;
  char line[2048];
  int i;
  char key[1024], value[1024];

  buf = dentdata;
  end = buf;
  *end = 0;

  for (i = 0; i < num_entities; i++) {
    ep = entities[i].epairs;
    if (!ep)
      continue; // ent got removed

    strcat(end, "{\n");
    end += 2;

    for (ep = entities[i].epairs; ep; ep = ep->next) {
      strcpy(key, ep->key);
      StripTrailing(key);
      strcpy(value, ep->value);
      StripTrailing(value);

      sprintf(line, "\"%s\" \"%s\"\n", key, value);
      strcat(end, line);
      end += strlen(line);
    }
    strcat(end, "}\n");
    end += 2;

    if (end > buf + MAX_MAP_ENTSTRING)
      Error("Entity text too long");
  }
  entdatasize = end - buf + 1;
}

void PrintEntity(entity_t *ent)
{
  epair_t *ep;

  printf("------- entity %p -------\n", ent);
  for (ep = ent->epairs; ep; ep = ep->next) {
    printf("%s = %s\n", ep->key, ep->value);
  }
}

void SetKeyValue(entity_t *ent, char *key, char *value)
{
  epair_t *ep;

  for (ep = ent->epairs; ep; ep = ep->next)
    if (!strcmp(ep->key, key)) {
      free(ep->value);
      ep->value = copystring(value);
      return;
    }
  ep = malloc(sizeof(*ep));
  ep->next = ent->epairs;
  ent->epairs = ep;
  ep->key = copystring(key);
  ep->value = copystring(value);
}

char *ValueForKey(entity_t *ent, char *key)
{
  epair_t *ep;

  for (ep = ent->epairs; ep; ep = ep->next)
    if (!strcmp(ep->key, key))
      return ep->value;
  return "";
}

vec_t FloatForKey(entity_t *ent, char *key)
{
  char *k;

  k = ValueForKey(ent, key);
  return atof(k);
}

void GetVectorForKey(entity_t *ent, char *key, vec3_t vec)
{
  char *k;
  double v1, v2, v3;

  k = ValueForKey(ent, key);
  // scanf into doubles, then assign, so it is vec_t size independent
  v1 = v2 = v3 = 0;
  sscanf(k, "%lf %lf %lf", &v1, &v2, &v3);
  vec[0] = v1;
  vec[1] = v2;
  vec[2] = v3;
}
