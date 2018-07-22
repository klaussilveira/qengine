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

// qdata.h

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>

#include "cmdlib.h"
#include "scriplib.h"
#include "mathlib.h"
#include "trilib.h"
#include "lbmlib.h"
#include "threads.h"
#include "l3dslib.h"
#include "bspfile.h"

void Cmd_Modelname(void);

void Cmd_Base(void);

void Cmd_Cd(void);

void Cmd_Origin(void);

void Cmd_ScaleUp(void);

void Cmd_Frame(void);

void Cmd_Modelname(void);

void Cmd_Skin(void);

void Cmd_Skinsize(void);

void FinishModel(void);

void Cmd_Inverse16Table(void);

void Cmd_SpriteName(void);

void Cmd_Load(void);

void Cmd_SpriteFrame(void);

void FinishSprite(void);

void Cmd_Grab(void);

void Cmd_Raw(void);

void Cmd_Mip(void);

void Cmd_Environment(void);

void Cmd_Colormap(void);

void Cmd_File(void);

void Cmd_Dir(void);

void Cmd_StartWad(void);

void Cmd_EndWad(void);

void Cmd_Mippal(void);

void Cmd_Mipdir(void);

void Cmd_Alphalight(void);

void Cmd_Video(void);

void RemapZero(byte *pixels, byte *palette, int width, int height);

void ReleaseFile(char *filename);

extern byte *byteimage, *lbmpalette;
extern int byteimagewidth, byteimageheight;

extern qboolean g_release;      // don't grab, copy output data to new tree
extern char g_releasedir[1024]; // c:\qengine\assets, etc
extern qboolean g_archive;      // don't grab, copy source data to new tree
extern qboolean do3ds;
extern char g_only[256];     // if set, only grab this cd
extern qboolean g_skipmodel; // set true when a cd is not g_only

extern char *trifileext;
