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

#define ON_EPSILON 0.1

typedef struct tnode_s
{
  int type;
  vec3_t normal;
  float dist;
  int children[2];
  int pad;
} tnode_t;

tnode_t *tnodes, *tnode_p;

/*
==============
MakeTnode

Converts the disk node structure into the efficient tracing structure
==============
*/
void MakeTnode(int nodenum)
{
  tnode_t *t;
  dplane_t *plane;
  int i;
  dnode_t *node;

  t = tnode_p++;

  node = dnodes + nodenum;
  plane = dplanes + node->planenum;

  t->type = plane->type;
  VectorCopy(plane->normal, t->normal);
  t->dist = plane->dist;

  for (i = 0; i < 2; i++) {
    if (node->children[i] < 0)
      t->children[i] = (dleafs[-node->children[i] - 1].contents & CONTENTS_SOLID) | (1 << 31);
    else {
      t->children[i] = tnode_p - tnodes;
      MakeTnode(node->children[i]);
    }
  }
}

/*
=============
MakeTnodes

Loads the node structure out of a .bsp file to be used for light occlusion
=============
*/
void MakeTnodes(dmodel_t *bm)
{
  // 32 byte align the structs
  tnodes = malloc((numnodes + 1) * sizeof(tnode_t));
  tnodes = (tnode_t *) (((int) tnodes + 31) & ~31);
  tnode_p = tnodes;

  MakeTnode(0);
}

//==========================================================

int TestLine_r(int node, vec3_t start, vec3_t stop)
{
  tnode_t *tnode;
  float front, back;
  vec3_t mid;
  float frac;
  int side;
  int r;

  if (node & (1 << 31))
    return node & ~(1 << 31); // leaf node

  tnode = &tnodes[node];
  switch (tnode->type) {
  case PLANE_X:
    front = start[0] - tnode->dist;
    back = stop[0] - tnode->dist;
    break;
  case PLANE_Y:
    front = start[1] - tnode->dist;
    back = stop[1] - tnode->dist;
    break;
  case PLANE_Z:
    front = start[2] - tnode->dist;
    back = stop[2] - tnode->dist;
    break;
  default:
    front = (start[0] * tnode->normal[0] + start[1] * tnode->normal[1] + start[2] * tnode->normal[2]) - tnode->dist;
    back = (stop[0] * tnode->normal[0] + stop[1] * tnode->normal[1] + stop[2] * tnode->normal[2]) - tnode->dist;
    break;
  }

  if (front >= -ON_EPSILON && back >= -ON_EPSILON)
    return TestLine_r(tnode->children[0], start, stop);

  if (front < ON_EPSILON && back < ON_EPSILON)
    return TestLine_r(tnode->children[1], start, stop);

  side = front < 0;

  frac = front / (front - back);

  mid[0] = start[0] + (stop[0] - start[0]) * frac;
  mid[1] = start[1] + (stop[1] - start[1]) * frac;
  mid[2] = start[2] + (stop[2] - start[2]) * frac;

  r = TestLine_r(tnode->children[side], start, mid);
  if (r)
    return r;
  return TestLine_r(tnode->children[!side], mid, stop);
}

int TestLine(vec3_t start, vec3_t stop)
{
  return TestLine_r(0, start, stop);
}

/*
==============================================================================

LINE TRACING

The major lighting operation is a point to point visibility test, performed
by recursive subdivision of the line by the BSP tree.

==============================================================================
*/

typedef struct
{
  vec3_t backpt;
  int side;
  int node;
} tracestack_t;

/*
==============
TestLine
==============
*/
qboolean _TestLine(vec3_t start, vec3_t stop)
{
  int node;
  float front, back;
  tracestack_t *tstack_p;
  int side;
  float frontx, fronty, frontz, backx, backy, backz;
  tracestack_t tracestack[64];
  tnode_t *tnode;

  frontx = start[0];
  fronty = start[1];
  frontz = start[2];
  backx = stop[0];
  backy = stop[1];
  backz = stop[2];

  tstack_p = tracestack;
  node = 0;

  while (1) {
    if (node == CONTENTS_SOLID) {
#if 0
            float	d1, d2, d3;

            d1 = backx - frontx;
            d2 = backy - fronty;
            d3 = backz - frontz;

            if (d1*d1 + d2*d2 + d3*d3 > 1)
#endif
      return false; // DONE!
    }

    while (node < 0) {
      // pop up the stack for a back side
      tstack_p--;
      if (tstack_p < tracestack)
        return true;
      node = tstack_p->node;

      // set the hit point for this plane

      frontx = backx;
      fronty = backy;
      frontz = backz;

      // go down the back side

      backx = tstack_p->backpt[0];
      backy = tstack_p->backpt[1];
      backz = tstack_p->backpt[2];

      node = tnodes[tstack_p->node].children[!tstack_p->side];
    }

    tnode = &tnodes[node];

    switch (tnode->type) {
    case PLANE_X:
      front = frontx - tnode->dist;
      back = backx - tnode->dist;
      break;
    case PLANE_Y:
      front = fronty - tnode->dist;
      back = backy - tnode->dist;
      break;
    case PLANE_Z:
      front = frontz - tnode->dist;
      back = backz - tnode->dist;
      break;
    default:
      front = (frontx * tnode->normal[0] + fronty * tnode->normal[1] + frontz * tnode->normal[2]) - tnode->dist;
      back = (backx * tnode->normal[0] + backy * tnode->normal[1] + backz * tnode->normal[2]) - tnode->dist;
      break;
    }

    if (front > -ON_EPSILON && back > -ON_EPSILON)
    //		if (front > 0 && back > 0)
    {
      node = tnode->children[0];
      continue;
    }

    if (front < ON_EPSILON && back < ON_EPSILON)
    //		if (front <= 0 && back <= 0)
    {
      node = tnode->children[1];
      continue;
    }

    side = front < 0;

    front = front / (front - back);

    tstack_p->node = node;
    tstack_p->side = side;
    tstack_p->backpt[0] = backx;
    tstack_p->backpt[1] = backy;
    tstack_p->backpt[2] = backz;

    tstack_p++;

    backx = frontx + front * (backx - frontx);
    backy = fronty + front * (backy - fronty);
    backz = frontz + front * (backz - frontz);

    node = tnode->children[side];
  }
}
