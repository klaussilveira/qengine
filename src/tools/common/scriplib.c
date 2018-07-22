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

// scriplib.c

#include "cmdlib.h"
#include "scriplib.h"

/*
=============================================================================

                                                PARSING STUFF

=============================================================================
*/

typedef struct
{
  char filename[1024];
  char *buffer, *script_p, *end_p;
  int line;
} script_t;

#define MAX_INCLUDES 8
script_t scriptstack[MAX_INCLUDES];
script_t *script;
int scriptline;

char token[MAXTOKEN];
qboolean endofscript;
qboolean tokenready; // only true if UnGetToken was just called

/*
==============
AddScriptToStack
==============
*/
void AddScriptToStack(char *filename)
{
  int size;

  script++;
  if (script == &scriptstack[MAX_INCLUDES])
    Error("script file exceeded MAX_INCLUDES");
  strcpy(script->filename, ExpandPath(filename));

  size = LoadFile(script->filename, (void **) &script->buffer);

  printf("entering %s\n", script->filename);

  script->line = 1;

  script->script_p = script->buffer;
  script->end_p = script->buffer + size;
}

/*
==============
LoadScriptFile
==============
*/
void LoadScriptFile(char *filename)
{
  script = scriptstack;
  AddScriptToStack(filename);

  endofscript = false;
  tokenready = false;
}

/*
==============
ParseFromMemory
==============
*/
void ParseFromMemory(char *buffer, int size)
{
  script = scriptstack;
  script++;
  if (script == &scriptstack[MAX_INCLUDES])
    Error("script file exceeded MAX_INCLUDES");
  strcpy(script->filename, "memory buffer");

  script->buffer = buffer;
  script->line = 1;
  script->script_p = script->buffer;
  script->end_p = script->buffer + size;

  endofscript = false;
  tokenready = false;
}

/*
==============
UnGetToken

Signals that the current token was not used, and should be reported
for the next GetToken.  Note that

GetToken (true);
UnGetToken ();
GetToken (false);

could cross a line boundary.
==============
*/
void UnGetToken(void)
{
  tokenready = true;
}

qboolean EndOfScript(qboolean crossline)
{
  if (!crossline)
    Error("Line %i is incomplete\n", scriptline);

  if (!strcmp(script->filename, "memory buffer")) {
    endofscript = true;
    return false;
  }

  free(script->buffer);
  if (script == scriptstack + 1) {
    endofscript = true;
    return false;
  }
  script--;
  scriptline = script->line;
  printf("returning to %s\n", script->filename);
  return GetToken(crossline);
}

/*
==============
GetToken
==============
*/
qboolean GetToken(qboolean crossline)
{
  char *token_p;

  if (tokenready) // is a token allready waiting?
  {
    tokenready = false;
    return true;
  }

  if (script->script_p >= script->end_p)
    return EndOfScript(crossline);

//
// skip space
//
skipspace:
  while (*script->script_p <= 32) {
    if (script->script_p >= script->end_p)
      return EndOfScript(crossline);
    if (*script->script_p++ == '\n') {
      if (!crossline)
        Error("Line %i is incomplete\n", scriptline);
      scriptline = script->line++;
    }
  }

  if (script->script_p >= script->end_p)
    return EndOfScript(crossline);

  // ; # // comments
  if (*script->script_p == ';' || *script->script_p == '#' ||
      (script->script_p[0] == '/' && script->script_p[1] == '/')) {
    if (!crossline)
      Error("Line %i is incomplete\n", scriptline);
    while (*script->script_p++ != '\n')
      if (script->script_p >= script->end_p)
        return EndOfScript(crossline);
    goto skipspace;
  }

  // /* */ comments
  if (script->script_p[0] == '/' && script->script_p[1] == '*') {
    if (!crossline)
      Error("Line %i is incomplete\n", scriptline);
    script->script_p += 2;
    while (script->script_p[0] != '*' && script->script_p[1] != '/') {
      script->script_p++;
      if (script->script_p >= script->end_p)
        return EndOfScript(crossline);
    }
    script->script_p += 2;
    goto skipspace;
  }

  //
  // copy token
  //
  token_p = token;

  if (*script->script_p == '"') {
    // quoted token
    script->script_p++;
    while (*script->script_p != '"') {
      *token_p++ = *script->script_p++;
      if (script->script_p == script->end_p)
        break;
      if (token_p == &token[MAXTOKEN])
        Error("Token too large on line %i\n", scriptline);
    }
    script->script_p++;
  } else // regular token
    while (*script->script_p > 32 && *script->script_p != ';') {
      *token_p++ = *script->script_p++;
      if (script->script_p == script->end_p)
        break;
      if (token_p == &token[MAXTOKEN])
        Error("Token too large on line %i\n", scriptline);
    }

  *token_p = 0;

  if (!strcmp(token, "$include")) {
    GetToken(false);
    AddScriptToStack(token);
    return GetToken(crossline);
  }

  return true;
}

/*
==============
TokenAvailable

Returns true if there is another token on the line
==============
*/
qboolean TokenAvailable(void)
{
  char *search_p;

  search_p = script->script_p;

  if (search_p >= script->end_p)
    return false;

  while (*search_p <= 32) {
    if (*search_p == '\n')
      return false;
    search_p++;
    if (search_p == script->end_p)
      return false;
  }

  if (*search_p == ';')
    return false;

  return true;
}
