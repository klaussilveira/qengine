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

// scriplib.h

#ifndef __CMDLIB__
#include "cmdlib.h"
#endif

#define MAXTOKEN 1024

extern char token[MAXTOKEN];
extern char *scriptbuffer, *script_p, *scriptend_p;
extern int grabbed;
extern int scriptline;
extern qboolean endofscript;

void LoadScriptFile(char *filename);

void ParseFromMemory(char *buffer, int size);

qboolean GetToken(qboolean crossline);

void UnGetToken(void);

qboolean TokenAvailable(void);
