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

#include "qbsp.h"

extern float subdivide_size;

char source[1024];
char name[1024];

vec_t microvolume = 1.0;
qboolean noprune;
qboolean glview;
qboolean nodetail;
qboolean fulldetail;
qboolean onlyents;
qboolean nomerge;
qboolean nowater;
qboolean nofill;
qboolean nocsg;
qboolean noweld;
qboolean noshare;
qboolean nosubdiv;
qboolean notjunc;
qboolean noopt;
qboolean leaktest;
qboolean verboseentities;

char outbase[32];

int block_xl = -8, block_xh = 7, block_yl = -8, block_yh = 7;

int entity_num;

node_t *block_nodes[10][10];

/*
============
BlockTree

============
*/
node_t *BlockTree(int xl, int yl, int xh, int yh)
{
  node_t *node;
  vec3_t normal;
  float dist;
  int mid;

  if (xl == xh && yl == yh) {
    node = block_nodes[xl + 5][yl + 5];
    if (!node) { // return an empty leaf
      node = AllocNode();
      node->planenum = PLANENUM_LEAF;
      node->contents = 0; // CONTENTS_SOLID;
      return node;
    }
    return node;
  }

  // create a seperator along the largest axis
  node = AllocNode();

  if (xh - xl > yh - yl) { // split x axis
    mid = xl + (xh - xl) / 2 + 1;
    normal[0] = 1;
    normal[1] = 0;
    normal[2] = 0;
    dist = mid * 1024;
    node->planenum = FindFloatPlane(normal, dist);
    node->children[0] = BlockTree(mid, yl, xh, yh);
    node->children[1] = BlockTree(xl, yl, mid - 1, yh);
  } else {
    mid = yl + (yh - yl) / 2 + 1;
    normal[0] = 0;
    normal[1] = 1;
    normal[2] = 0;
    dist = mid * 1024;
    node->planenum = FindFloatPlane(normal, dist);
    node->children[0] = BlockTree(xl, mid, xh, yh);
    node->children[1] = BlockTree(xl, yl, xh, mid - 1);
  }

  return node;
}

/*
============
ProcessBlock_Thread

============
*/
int brush_start, brush_end;

void ProcessBlock_Thread(int blocknum)
{
  int xblock, yblock;
  vec3_t mins, maxs;
  bspbrush_t *brushes;
  tree_t *tree;
  node_t *node;

  yblock = block_yl + blocknum / (block_xh - block_xl + 1);
  xblock = block_xl + blocknum % (block_xh - block_xl + 1);

  qprintf("############### block %2i,%2i ###############\n", xblock, yblock);

  mins[0] = xblock * 1024;
  mins[1] = yblock * 1024;
  mins[2] = -4096;
  maxs[0] = (xblock + 1) * 1024;
  maxs[1] = (yblock + 1) * 1024;
  maxs[2] = 4096;

  // the makelist and chopbrushes could be cached between the passes...
  brushes = MakeBspBrushList(brush_start, brush_end, mins, maxs);
  if (!brushes) {
    node = AllocNode();
    node->planenum = PLANENUM_LEAF;
    node->contents = CONTENTS_SOLID;
    block_nodes[xblock + 5][yblock + 5] = node;
    return;
  }

  if (!nocsg)
    brushes = ChopBrushes(brushes);

  tree = BrushBSP(brushes, mins, maxs);

  block_nodes[xblock + 5][yblock + 5] = tree->headnode;
}

/*
============
ProcessWorldModel

============
*/
void ProcessWorldModel(void)
{
  entity_t *e;
  tree_t *tree;
  qboolean leaked;
  qboolean optimize;

  e = &entities[entity_num];

  brush_start = e->firstbrush;
  brush_end = brush_start + e->numbrushes;
  leaked = false;

  //
  // perform per-block operations
  //
  if (block_xh * 1024 > map_maxs[0])
    block_xh = floor(map_maxs[0] / 1024.0);
  if ((block_xl + 1) * 1024 < map_mins[0])
    block_xl = floor(map_mins[0] / 1024.0);
  if (block_yh * 1024 > map_maxs[1])
    block_yh = floor(map_maxs[1] / 1024.0);
  if ((block_yl + 1) * 1024 < map_mins[1])
    block_yl = floor(map_mins[1] / 1024.0);

  if (block_xl < -4)
    block_xl = -4;
  if (block_yl < -4)
    block_yl = -4;
  if (block_xh > 3)
    block_xh = 3;
  if (block_yh > 3)
    block_yh = 3;

  for (optimize = false; optimize <= true; optimize++) {
    qprintf("--------------------------------------------\n");

    RunThreadsOnIndividual((block_xh - block_xl + 1) * (block_yh - block_yl + 1), !verbose, ProcessBlock_Thread);

    //
    // build the division tree
    // oversizing the blocks guarantees that all the boundaries
    // will also get nodes.
    //

    qprintf("--------------------------------------------\n");

    tree = AllocTree();
    tree->headnode = BlockTree(block_xl - 1, block_yl - 1, block_xh + 1, block_yh + 1);

    tree->mins[0] = (block_xl) *1024;
    tree->mins[1] = (block_yl) *1024;
    tree->mins[2] = map_mins[2] - 8;

    tree->maxs[0] = (block_xh + 1) * 1024;
    tree->maxs[1] = (block_yh + 1) * 1024;
    tree->maxs[2] = map_maxs[2] + 8;

    //
    // perform the global operations
    //
    MakeTreePortals(tree);

    if (FloodEntities(tree))
      FillOutside(tree->headnode);
    else {
      printf("**** leaked ****\n");
      leaked = true;
      LeakFile(tree);
      if (leaktest) {
        printf("--- MAP LEAKED ---\n");
        exit(0);
      }
    }

    MarkVisibleSides(tree, brush_start, brush_end);
    if (noopt || leaked)
      break;
    if (!optimize) {
      FreeTree(tree);
    }
  }

  FloodAreas(tree);
  if (glview)
    WriteGLView(tree, source);
  MakeFaces(tree->headnode);
  FixTjuncs(tree->headnode);

  if (!noprune)
    PruneNodes(tree->headnode);

  WriteBSP(tree->headnode);

  if (!leaked)
    WritePortalFile(tree);

  FreeTree(tree);
}

/*
============
ProcessSubModel

============
*/
void ProcessSubModel(void)
{
  entity_t *e;
  int start, end;
  tree_t *tree;
  bspbrush_t *list;
  vec3_t mins, maxs;

  e = &entities[entity_num];

  start = e->firstbrush;
  end = start + e->numbrushes;

  mins[0] = mins[1] = mins[2] = -4096;
  maxs[0] = maxs[1] = maxs[2] = 4096;
  list = MakeBspBrushList(start, end, mins, maxs);
  if (!nocsg)
    list = ChopBrushes(list);
  tree = BrushBSP(list, mins, maxs);
  MakeTreePortals(tree);
  MarkVisibleSides(tree, start, end);
  MakeFaces(tree->headnode);
  FixTjuncs(tree->headnode);
  WriteBSP(tree->headnode);
  FreeTree(tree);
}

/*
============
ProcessModels
============
*/
void ProcessModels(void)
{
  BeginBSPFile();

  for (entity_num = 0; entity_num < num_entities; entity_num++) {
    if (!entities[entity_num].numbrushes)
      continue;

    qprintf("############### model %i ###############\n", nummodels);
    BeginModel();
    if (entity_num == 0)
      ProcessWorldModel();
    else
      ProcessSubModel();
    EndModel();

    if (!verboseentities)
      verbose = false; // don't bother printing submodels
  }

  EndBSPFile();
}

/*
============
main
============
*/
int main(int argc, char **argv)
{
  int i;
  double start, end;
  char path[1024];

  printf("---- qbsp3 ----\n");

  for (i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-threads")) {
      numthreads = atoi(argv[i + 1]);
      i++;
    } else if (!strcmp(argv[i], "-glview")) {
      glview = true;
    } else if (!strcmp(argv[i], "-v")) {
      printf("verbose = true\n");
      verbose = true;
    } else if (!strcmp(argv[i], "-draw")) {
      printf("drawflag = true\n");
      drawflag = true;
    } else if (!strcmp(argv[i], "-noweld")) {
      printf("noweld = true\n");
      noweld = true;
    } else if (!strcmp(argv[i], "-nocsg")) {
      printf("nocsg = true\n");
      nocsg = true;
    } else if (!strcmp(argv[i], "-noshare")) {
      printf("noshare = true\n");
      noshare = true;
    } else if (!strcmp(argv[i], "-notjunc")) {
      printf("notjunc = true\n");
      notjunc = true;
    } else if (!strcmp(argv[i], "-nowater")) {
      printf("nowater = true\n");
      nowater = true;
    } else if (!strcmp(argv[i], "-noopt")) {
      printf("noopt = true\n");
      noopt = true;
    } else if (!strcmp(argv[i], "-noprune")) {
      printf("noprune = true\n");
      noprune = true;
    } else if (!strcmp(argv[i], "-nofill")) {
      printf("nofill = true\n");
      nofill = true;
    } else if (!strcmp(argv[i], "-nomerge")) {
      printf("nomerge = true\n");
      nomerge = true;
    } else if (!strcmp(argv[i], "-nosubdiv")) {
      printf("nosubdiv = true\n");
      nosubdiv = true;
    } else if (!strcmp(argv[i], "-nodetail")) {
      printf("nodetail = true\n");
      nodetail = true;
    } else if (!strcmp(argv[i], "-fulldetail")) {
      printf("fulldetail = true\n");
      fulldetail = true;
    } else if (!strcmp(argv[i], "-onlyents")) {
      printf("onlyents = true\n");
      onlyents = true;
    } else if (!strcmp(argv[i], "-micro")) {
      microvolume = atof(argv[i + 1]);
      printf("microvolume = %f\n", microvolume);
      i++;
    } else if (!strcmp(argv[i], "-leaktest")) {
      printf("leaktest = true\n");
      leaktest = true;
    } else if (!strcmp(argv[i], "-verboseentities")) {
      printf("verboseentities = true\n");
      verboseentities = true;
    } else if (!strcmp(argv[i], "-chop")) {
      subdivide_size = atof(argv[i + 1]);
      printf("subdivide_size = %f\n", subdivide_size);
      i++;
    } else if (!strcmp(argv[i], "-block")) {
      block_xl = block_xh = atoi(argv[i + 1]);
      block_yl = block_yh = atoi(argv[i + 2]);
      printf("block: %i,%i\n", block_xl, block_yl);
      i += 2;
    } else if (!strcmp(argv[i], "-blocks")) {
      block_xl = atoi(argv[i + 1]);
      block_yl = atoi(argv[i + 2]);
      block_xh = atoi(argv[i + 3]);
      block_yh = atoi(argv[i + 4]);
      printf("blocks: %i,%i to %i,%i\n", block_xl, block_yl, block_xh, block_yh);
      i += 4;
    } else if (!strcmp(argv[i], "-tmpout")) {
      strcpy(outbase, "/tmp");
    } else if (argv[i][0] == '-')
      Error("Unknown option \"%s\"", argv[i]);
    else
      break;
  }

  if (i != argc - 1)
    Error("usage: qbsp3 [options] mapfile");

  start = I_FloatTime();

  ThreadSetDefault();
  numthreads = 1; // multiple threads aren't helping...
  SetQdirFromPath(argv[i]);

  strcpy(source, ExpandArg(argv[i]));
  StripExtension(source);

  // delete portal and line files
  sprintf(path, "%s.prt", source);
  remove(path);
  sprintf(path, "%s.lin", source);
  remove(path);

  strcpy(name, ExpandArg(argv[i]));
  DefaultExtension(name, ".map"); // might be .reg

  //
  // if onlyents, just grab the entites and resave
  //
  if (onlyents) {
    char out[1024];

    sprintf(out, "%s.bsp", source);
    LoadBSPFile(out);
    num_entities = 0;

    LoadMapFile(name);
    SetModelNumbers();
    SetLightStyles();

    UnparseEntities();

    WriteBSPFile(out);
  } else {
    //
    // start from scratch
    //
    LoadMapFile(name);
    SetModelNumbers();
    SetLightStyles();

    ProcessModels();
  }

  end = I_FloatTime();
  printf("%5.0f seconds elapsed\n", end - start);

  return 0;
}
