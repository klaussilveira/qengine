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
 *
 * =======================================================================
 *
 * This is the refresher dependend video menu. If you add a new
 * refresher this menu must be altered.
 *
 * =======================================================================
 */

#include "../../client/header/client.h"
#include "../../client/menu/header/qmenu.h"

extern void M_ForceMenuOff(void);

static cvar_t *r_mode;
static cvar_t *r_hudscale;
static cvar_t *r_consolescale;
static cvar_t *r_menuscale;
static cvar_t *crosshair_scale;
static cvar_t *fov;
extern cvar_t *scr_viewsize;
extern cvar_t *vid_gamma;
extern cvar_t *vid_fullscreen;
static cvar_t *r_vsync;

static menuframework_s s_video_menu;

static menulist_s s_mode_list;
static menulist_s s_uiscale_list;
static menuslider_s s_brightness_slider;
static menuslider_s s_fov_slider;
static menulist_s s_fs_box;
static menulist_s s_vsync_list;
static menuaction_s s_defaults_action;
static menuaction_s s_apply_action;

static int GetCustomValue(menulist_s *list)
{
  static menulist_s *last;
  static int i;

  if (list != last) {
    last = list;
    i = list->curvalue;
    do {
      i++;
    } while (list->itemnames[i]);
    i--;
  }

  return i;
}

static void onBrightnessChange(void *self)
{
  menuslider_s *slider = (menuslider_s *) self;

  float gamma = slider->curvalue / 10.0;
  Cvar_SetValue("vid_gamma", gamma);
}

static void onFieldOfViewChange(void *self)
{
  menuslider_s *slider = (menuslider_s *) self;
  Cvar_SetValue("fov", slider->curvalue);
}

static void onResetDefaults()
{
  VID_MenuInit();
}

static void onApplyChanges()
{
  qboolean restart = false;

  /* custom mode */
  if (s_mode_list.curvalue != GetCustomValue(&s_mode_list)) {
    /* Restarts automatically */
    Cvar_SetValue("r_mode", s_mode_list.curvalue);
  } else {
    /* Restarts automatically */
    Cvar_SetValue("r_mode", -1);
  }

  /* UI scaling */
  if (s_uiscale_list.curvalue == 0) {
    Cvar_SetValue("r_hudscale", -1);
  } else if (s_uiscale_list.curvalue < GetCustomValue(&s_uiscale_list)) {
    Cvar_SetValue("r_hudscale", s_uiscale_list.curvalue);
  }

  if (s_uiscale_list.curvalue != GetCustomValue(&s_uiscale_list)) {
    Cvar_SetValue("r_consolescale", r_hudscale->value);
    Cvar_SetValue("r_menuscale", r_hudscale->value);
    Cvar_SetValue("crosshair_scale", r_hudscale->value);
  }

  /* Restarts automatically */
  Cvar_SetValue("vid_fullscreen", s_fs_box.curvalue);

  /* vertical sync */
  if (r_vsync->value != s_vsync_list.curvalue) {
    Cvar_SetValue("r_vsync", s_vsync_list.curvalue);
    restart = true;
  }

  if (restart) {
    Cbuf_AddText("vid_restart\n");
  }

  M_ForceMenuOff();
}

void VID_MenuInit(void)
{
  int y = 0;

  static const char *resolutions[] = {"[320 240   ]", "[400 300   ]",
                                      "[512 384   ]", "[640 400   ]",
                                      "[640 480   ]", "[800 500   ]",
                                      "[800 600   ]", "[960 720   ]",
                                      "[1024 480  ]", "[1024 640  ]",
                                      "[1024 768  ]", "[1152 768  ]",
                                      "[1152 864  ]", "[1280 800  ]",
                                      "[1280 720  ]", "[1280 960  ]",
                                      "[1280 1024 ]", "[1366 768  ]",
                                      "[1440 900  ]", "[1600 1200 ]",
                                      "[1680 1050 ]", "[1920 1080 ]",
                                      "[1920 1200 ]", "[2048 1536 ]",
                                      "[2560 1080 ]", "[2560 1440 ]",
                                      "[2560 1600 ]", "[3440 1440 ]",
                                      "[3840 1600 ]", "[3840 2160 ]",
                                      "[4096 2160 ]", "[5120 2880 ]",
                                      "[custom    ]", 0};

  static const char *uiscale_names[] = {"auto", "1x", "2x", "3x", "4x", "5x", "6x", "custom", 0};

  static const char *yesno_names[] = {"no", "yes", 0};

  static const char *fullscreen_names[] = {"no", "keep resolution", "switch resolution", 0};

  if (!r_mode) {
    r_mode = Cvar_Get("r_mode", "4", 0);
  }

  if (!r_hudscale) {
    r_hudscale = Cvar_Get("r_hudscale", "-1", CVAR_ARCHIVE);
  }

  if (!r_consolescale) {
    r_consolescale = Cvar_Get("r_consolescale", "-1", CVAR_ARCHIVE);
  }

  if (!r_menuscale) {
    r_menuscale = Cvar_Get("r_menuscale", "-1", CVAR_ARCHIVE);
  }

  if (!crosshair_scale) {
    crosshair_scale = Cvar_Get("crosshair_scale", "-1", CVAR_ARCHIVE);
  }

  if (!fov) {
    fov = Cvar_Get("fov", "90", CVAR_USERINFO | CVAR_ARCHIVE);
  }

  if (!vid_gamma) {
    vid_gamma = Cvar_Get("vid_gamma", "1.2", CVAR_ARCHIVE);
  }

  if (!r_vsync) {
    r_vsync = Cvar_Get("r_vsync", "1", CVAR_ARCHIVE);
  }

  s_mode_list.generic.type = MTYPE_SPINCONTROL;
  s_mode_list.generic.name = "video mode";
  s_mode_list.generic.x = 0;
  s_mode_list.generic.y = (y += 10);
  s_mode_list.itemnames = resolutions;

  if (r_mode->value >= 0) {
    s_mode_list.curvalue = r_mode->value;
  } else {
    s_mode_list.curvalue = GetCustomValue(&s_mode_list);
  }

  s_brightness_slider.generic.type = MTYPE_SLIDER;
  s_brightness_slider.generic.name = "brightness";
  s_brightness_slider.generic.x = 0;
  s_brightness_slider.generic.y = (y += 20);
  s_brightness_slider.generic.callback = onBrightnessChange;
  s_brightness_slider.minvalue = 1;
  s_brightness_slider.maxvalue = 20;
  s_brightness_slider.curvalue = vid_gamma->value * 10;

  s_fov_slider.generic.type = MTYPE_SLIDER;
  s_fov_slider.generic.x = 0;
  s_fov_slider.generic.y = (y += 10);
  s_fov_slider.generic.name = "field of view";
  s_fov_slider.generic.callback = onFieldOfViewChange;
  s_fov_slider.minvalue = 60;
  s_fov_slider.maxvalue = 120;
  s_fov_slider.curvalue = fov->value;

  s_uiscale_list.generic.type = MTYPE_SPINCONTROL;
  s_uiscale_list.generic.name = "ui scale";
  s_uiscale_list.generic.x = 0;
  s_uiscale_list.generic.y = (y += 10);
  s_uiscale_list.itemnames = uiscale_names;
  if (r_hudscale->value != r_consolescale->value || r_hudscale->value != r_menuscale->value ||
      r_hudscale->value != crosshair_scale->value) {
    s_uiscale_list.curvalue = GetCustomValue(&s_uiscale_list);
  } else if (r_hudscale->value < 0) {
    s_uiscale_list.curvalue = 0;
  } else if (r_hudscale->value > 0 && r_hudscale->value < GetCustomValue(&s_uiscale_list) &&
             r_hudscale->value == (int) r_hudscale->value) {
    s_uiscale_list.curvalue = r_hudscale->value;
  } else {
    s_uiscale_list.curvalue = GetCustomValue(&s_uiscale_list);
  }

  s_fs_box.generic.type = MTYPE_SPINCONTROL;
  s_fs_box.generic.name = "fullscreen";
  s_fs_box.generic.x = 0;
  s_fs_box.generic.y = (y += 10);
  s_fs_box.itemnames = fullscreen_names;
  s_fs_box.curvalue = (int) vid_fullscreen->value;

  s_vsync_list.generic.type = MTYPE_SPINCONTROL;
  s_vsync_list.generic.name = "vertical sync";
  s_vsync_list.generic.x = 0;
  s_vsync_list.generic.y = (y += 10);
  s_vsync_list.itemnames = yesno_names;
  s_vsync_list.curvalue = (r_vsync->value != 0);

  s_defaults_action.generic.type = MTYPE_ACTION;
  s_defaults_action.generic.name = "reset to default";
  s_defaults_action.generic.x = 0;
  s_defaults_action.generic.y = (y += 20);
  s_defaults_action.generic.callback = onResetDefaults;

  s_apply_action.generic.type = MTYPE_ACTION;
  s_apply_action.generic.name = "apply";
  s_apply_action.generic.x = 0;
  s_apply_action.generic.y = (y += 10);
  s_apply_action.generic.callback = onApplyChanges;

  s_video_menu.x = viddef.width * 0.50;
  s_video_menu.nitems = 0;

  Menu_AddItem(&s_video_menu, (void *) &s_mode_list);
  Menu_AddItem(&s_video_menu, (void *) &s_brightness_slider);
  Menu_AddItem(&s_video_menu, (void *) &s_fov_slider);
  Menu_AddItem(&s_video_menu, (void *) &s_uiscale_list);
  Menu_AddItem(&s_video_menu, (void *) &s_fs_box);
  Menu_AddItem(&s_video_menu, (void *) &s_vsync_list);
  Menu_AddItem(&s_video_menu, (void *) &s_defaults_action);
  Menu_AddItem(&s_video_menu, (void *) &s_apply_action);

  Menu_Center(&s_video_menu);
  s_video_menu.x -= 8;
}

void VID_MenuDraw(void)
{
  int w, h;
  float scale = SCR_GetMenuScale();

  /* draw the banner */
  Draw_GetPicSize(&w, &h, "m_banner_video");
  Draw_PicScaled(viddef.width / 2 - (w * scale) / 2, viddef.height / 2 - (110 * scale), "m_banner_video", scale);

  /* move cursor to a reasonable starting position */
  Menu_AdjustCursor(&s_video_menu, 1);

  /* draw the menu */
  Menu_Draw(&s_video_menu);
}

const char *VID_MenuKey(int key)
{
  extern void M_PopMenu(void);

  menuframework_s *m = &s_video_menu;
  static const char *sound = "misc/menu1.wav";
  int menu_key = Key_GetMenuKey(key);

  switch (menu_key) {
  case K_ESCAPE:
    M_PopMenu();
    return NULL;
  case K_UPARROW:
    m->cursor--;
    Menu_AdjustCursor(m, -1);
    break;
  case K_DOWNARROW:
    m->cursor++;
    Menu_AdjustCursor(m, 1);
    break;
  case K_LEFTARROW:
    Menu_SlideItem(m, -1);
    break;
  case K_RIGHTARROW:
    Menu_SlideItem(m, 1);
    break;
  case K_ENTER:
    Menu_SelectItem(m);
    break;
  }

  return sound;
}
