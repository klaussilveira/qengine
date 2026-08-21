/*
 * Copyright (C) 1997-2001 Id Software, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include "../../client/header/client.h"
#include "../../client/renderer/header/local.h"

qboolean VID_LoadRefresh(void);

/* Console variables that we need to access from this module */
cvar_t *vid_gamma;
cvar_t *vid_fullscreen;

/* Global variables used internally by this module */
viddef_t viddef; /* global video state; used by other modules */

static qboolean vid_reload_requested;

#define MAXPRINTMSG 4096

/*
 * Console command to re-start the video mode and refresh.
 */
void VID_Restart_f(void)
{
  vid_reload_requested = true;
}

void VID_NewWindow(int width, int height)
{
  viddef.width = width;
  viddef.height = height;
}

/*
 * This function gets called once just before drawing each frame, and
 * it's sole purpose in life is to check to see if any of the video mode
 * parameters have changed, and if they have to update the refresh
 * and/or video mode to match.
 */
void VID_CheckChanges(void)
{
  if (vid_reload_requested) {
    vid_reload_requested = false;

    S_StopAllSounds();

    /* refresh has changed */
    cl.refresh_prepped = false;
    cls.disable_screen = true;

    VID_LoadRefresh();

    cls.disable_screen = false;
  }
}

void VID_Init(void)
{
  /* Create the video variables so we know how to start the graphics drivers */
  vid_fullscreen = Cvar_Get("vid_fullscreen", "0", CVAR_ARCHIVE);
  vid_gamma = Cvar_Get("vid_gamma", "1.2", CVAR_ARCHIVE);

  /* Add some console commands that we want to handle */
  Cmd_AddCommand("vid_restart", VID_Restart_f);

  /* Start the graphics mode and load refresh DLL */
  VID_LoadRefresh();
}

void Key_MarkAllUp(void);

qboolean VID_LoadRefresh(void)
{
  if (!RE_Init()) {
    VID_Shutdown();
    Com_Printf("Failed loading renderer\n");

    return false;
  }

  Key_MarkAllUp();
  Com_Printf("Successfully loaded renderer\n");

  return true;
}

void VID_Shutdown(void)
{
  RE_Shutdown();
}

// ======== wrappers for functions from refresh lib ========

void R_BeginRegistration(char *map)
{
  RE_BeginRegistration(map);
}

struct model_s *R_RegisterModel(char *name)
{
  return RE_RegisterModel(name);
}

struct image_s *R_RegisterSkin(char *name)
{
  return RE_RegisterSkin(name);
}

void R_SetSky(char *name, float rotate, vec3_t axis)
{
  RE_SetSky(name, rotate, axis);
}

void R_EndRegistration(void)
{
  RE_EndRegistration();
}

void R_RenderFrame(refdef_t *fd)
{
  RE_RenderFrame(fd);
}

struct image_s *Draw_FindPic(char *name)
{
  return RE_Draw_FindPic(name);
}

void Draw_GetPicSize(int *w, int *h, char *name)
{
  RE_Draw_GetPicSize(w, h, name);
}

void Draw_StretchPic(int x, int y, int w, int h, char *name)
{
  RE_Draw_StretchPic(x, y, w, h, name);
}

void Draw_PicScaled(int x, int y, char *pic, float factor)
{
  RE_Draw_PicScaled(x, y, pic, factor);
}

void Draw_CharScaled(int x, int y, int num, float scale)
{
  RE_Draw_CharScaled(x, y, num, scale);
}

void Draw_TileClear(int x, int y, int w, int h, char *name)
{
  RE_Draw_TileClear(x, y, w, h, name);
}

void Draw_Fill(int x, int y, int w, int h, int c)
{
  RE_Draw_Fill(x, y, w, h, c);
}

void Draw_FadeScreen(void)
{
  RE_Draw_FadeScreen();
}

void Draw_StretchRaw(int x, int y, int w, int h, int cols, int rows, byte *data)
{
  RE_Draw_StretchRaw(x, y, w, h, cols, rows, data);
}

void R_SetPalette(const unsigned char *palette)
{
  RE_SetPalette(palette);
}

void R_BeginFrame(float camera_separation)
{
  RE_BeginFrame(camera_separation);
}

void R_EndFrame(void)
{
  RE_EndFrame();
}

qboolean R_IsVSyncActive(void)
{
  return RE_IsVsyncActive();
}
