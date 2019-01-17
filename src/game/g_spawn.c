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
 * Item spawning.
 *
 * =======================================================================
 */

#include "../server/header/server.h"

typedef struct
{
  char *name;

  void (*spawn)(edict_t *ent);
} spawn_t;

void SP_item_health(edict_t *self);

void SP_item_health_small(edict_t *self);

void SP_item_health_large(edict_t *self);

void SP_item_health_mega(edict_t *self);

void SP_info_player_start(edict_t *ent);

void SP_info_player_deathmatch(edict_t *ent);

void SP_info_player_coop(edict_t *ent);

void SP_info_player_intermission(edict_t *ent);

void SP_func_plat(edict_t *ent);

void SP_func_rotating(edict_t *ent);

void SP_func_button(edict_t *ent);

void SP_func_door(edict_t *ent);

void SP_func_door_secret(edict_t *ent);

void SP_func_door_rotating(edict_t *ent);

void SP_func_water(edict_t *ent);

void SP_func_train(edict_t *ent);

void SP_func_wall(edict_t *self);

void SP_func_object(edict_t *self);

void SP_func_explosive(edict_t *self);

void SP_func_timer(edict_t *self);

void SP_func_areaportal(edict_t *ent);

void SP_func_killbox(edict_t *ent);

void SP_trigger_always(edict_t *ent);

void SP_trigger_once(edict_t *ent);

void SP_trigger_multiple(edict_t *ent);

void SP_trigger_relay(edict_t *ent);

void SP_trigger_push(edict_t *ent);

void SP_trigger_hurt(edict_t *ent);

void SP_trigger_key(edict_t *ent);

void SP_trigger_counter(edict_t *ent);

void SP_trigger_elevator(edict_t *ent);

void SP_trigger_gravity(edict_t *ent);

void SP_trigger_monsterjump(edict_t *ent);

void SP_target_temp_entity(edict_t *ent);

void SP_target_speaker(edict_t *ent);

void SP_target_explosion(edict_t *ent);

void SP_target_changelevel(edict_t *ent);

void SP_target_secret(edict_t *ent);

void SP_target_goal(edict_t *ent);

void SP_target_splash(edict_t *ent);

void SP_target_spawner(edict_t *ent);

void SP_target_blaster(edict_t *ent);

void SP_target_crosslevel_trigger(edict_t *ent);

void SP_target_crosslevel_target(edict_t *ent);

void SP_target_laser(edict_t *self);

void SP_target_lightramp(edict_t *self);

void SP_target_earthquake(edict_t *ent);

void SP_target_character(edict_t *ent);

void SP_target_string(edict_t *ent);

void SP_worldspawn(edict_t *ent);

void SP_light(edict_t *self);

void SP_info_null(edict_t *self);

void SP_info_notnull(edict_t *self);

void SP_path_corner(edict_t *self);

void SP_point_combat(edict_t *self);

void SP_misc_explobox(edict_t *self);

void SP_misc_teleporter(edict_t *self);

void SP_misc_teleporter_dest(edict_t *self);

void SP_monster_infantry(edict_t *self);

void SP_monster_soldier_light(edict_t *self);

void SP_monster_soldier(edict_t *self);

void SP_monster_soldier_ss(edict_t *self);

spawn_t spawns[] = {{"item_health", SP_item_health},
                    {"item_health_small", SP_item_health_small},
                    {"item_health_large", SP_item_health_large},
                    {"item_health_mega", SP_item_health_mega},

                    {"info_player_start", SP_info_player_start},
                    {"info_player_deathmatch", SP_info_player_deathmatch},
                    {"info_player_coop", SP_info_player_coop},
                    {"info_player_intermission", SP_info_player_intermission},

                    {"func_plat", SP_func_plat},
                    {"func_button", SP_func_button},
                    {"func_door", SP_func_door},
                    {"func_door_secret", SP_func_door_secret},
                    {"func_door_rotating", SP_func_door_rotating},
                    {"func_rotating", SP_func_rotating},
                    {"func_train", SP_func_train},
                    {"func_water", SP_func_water},
                    {"func_areaportal", SP_func_areaportal},
                    {"func_wall", SP_func_wall},
                    {"func_object", SP_func_object},
                    {"func_timer", SP_func_timer},
                    {"func_explosive", SP_func_explosive},
                    {"func_killbox", SP_func_killbox},

                    {"trigger_always", SP_trigger_always},
                    {"trigger_once", SP_trigger_once},
                    {"trigger_multiple", SP_trigger_multiple},
                    {"trigger_relay", SP_trigger_relay},
                    {"trigger_push", SP_trigger_push},
                    {"trigger_hurt", SP_trigger_hurt},
                    {"trigger_key", SP_trigger_key},
                    {"trigger_counter", SP_trigger_counter},
                    {"trigger_elevator", SP_trigger_elevator},
                    {"trigger_gravity", SP_trigger_gravity},
                    {"trigger_monsterjump", SP_trigger_monsterjump},

                    {"target_temp_entity", SP_target_temp_entity},
                    {"target_speaker", SP_target_speaker},
                    {"target_explosion", SP_target_explosion},
                    {"target_changelevel", SP_target_changelevel},
                    {"target_secret", SP_target_secret},
                    {"target_goal", SP_target_goal},
                    {"target_splash", SP_target_splash},
                    {"target_spawner", SP_target_spawner},
                    {"target_blaster", SP_target_blaster},
                    {"target_crosslevel_trigger", SP_target_crosslevel_trigger},
                    {"target_crosslevel_target", SP_target_crosslevel_target},
                    {"target_laser", SP_target_laser},
                    {"target_lightramp", SP_target_lightramp},
                    {"target_earthquake", SP_target_earthquake},
                    {"target_character", SP_target_character},
                    {"target_string", SP_target_string},

                    {"worldspawn", SP_worldspawn},

                    {"light", SP_light},
                    {"info_null", SP_info_null},
                    {"func_group", SP_info_null},
                    {"info_notnull", SP_info_notnull},
                    {"path_corner", SP_path_corner},
                    {"point_combat", SP_point_combat},

                    {"misc_explobox", SP_misc_explobox},
                    {"misc_teleporter", SP_misc_teleporter},
                    {"misc_teleporter_dest", SP_misc_teleporter_dest},

                    {"monster_infantry", SP_monster_infantry},
                    {"monster_soldier_light", SP_monster_soldier_light},
                    {"monster_soldier", SP_monster_soldier},
                    {"monster_soldier_ss", SP_monster_soldier_ss},

                    {NULL, NULL}};

/*
 * Finds the spawn function for
 * the entity and calls it
 */
void ED_CallSpawn(edict_t *ent)
{
  spawn_t *s;
  gitem_t *item;
  int i;

  if (!ent) {
    return;
  }

  if (!ent->classname) {
    PF_dprintf("ED_CallSpawn: NULL classname\n");
    G_FreeEdict(ent);
    return;
  }

  /* check item spawn functions */
  for (i = 0, item = itemlist; i < game.num_items; i++, item++) {
    if (!item->classname) {
      continue;
    }

    if (!strcmp(item->classname, ent->classname)) {
      /* found it */
      SpawnItem(ent, item);
      return;
    }
  }

  /* check normal spawn functions */
  for (s = spawns; s->name; s++) {
    if (!strcmp(s->name, ent->classname)) {
      /* found it */
      s->spawn(ent);
      return;
    }
  }

  PF_dprintf("%s doesn't have a spawn function\n", ent->classname);
}

char *ED_NewString(const char *string)
{
  char *newb, *new_p;
  int i, l;

  if (!string) {
    return NULL;
  }

  l = strlen(string) + 1;

  newb = Z_TagMalloc(l, TAG_LEVEL);

  new_p = newb;

  for (i = 0; i < l; i++) {
    if ((string[i] == '\\') && (i < l - 1)) {
      i++;

      if (string[i] == 'n') {
        *new_p++ = '\n';
      } else {
        *new_p++ = '\\';
      }
    } else {
      *new_p++ = string[i];
    }
  }

  return newb;
}

/*
 * Takes a key/value pair and sets
 * the binary values in an edict
 */
void ED_ParseField(const char *key, const char *value, edict_t *ent)
{
  field_t *f;
  byte *b;
  float v;
  vec3_t vec;

  if (!key || !value) {
    return;
  }

  for (f = fields; f->name; f++) {
    if (!(f->flags & FFL_NOSPAWN) && !Q_strcasecmp(f->name, (char *) key)) {
      /* found it */
      if (f->flags & FFL_SPAWNTEMP) {
        b = (byte *) &st;
      } else {
        b = (byte *) ent;
      }

      switch (f->type) {
      case F_LSTRING:
        *(char **) (b + f->ofs) = ED_NewString(value);
        break;
      case F_VECTOR:
        sscanf(value, "%f %f %f", &vec[0], &vec[1], &vec[2]);
        ((float *) (b + f->ofs))[0] = vec[0];
        ((float *) (b + f->ofs))[1] = vec[1];
        ((float *) (b + f->ofs))[2] = vec[2];
        break;
      case F_INT:
        *(int *) (b + f->ofs) = (int) strtol(value, (char **) NULL, 10);
        break;
      case F_FLOAT:
        *(float *) (b + f->ofs) = (float) strtod(value, (char **) NULL);
        break;
      case F_ANGLEHACK:
        v = (float) strtod(value, (char **) NULL);
        ((float *) (b + f->ofs))[0] = 0;
        ((float *) (b + f->ofs))[1] = v;
        ((float *) (b + f->ofs))[2] = 0;
        break;
      case F_IGNORE:
        break;
      default:
        break;
      }

      return;
    }
  }

  PF_dprintf("%s is not a field\n", key);
}

/*
 * Parses an edict out of the given string,
 * returning the new position ed should be
 * a properly initialized empty edict.
 */
char *ED_ParseEdict(char *data, edict_t *ent)
{
  qboolean init;
  char keyname[256];
  const char *com_token;

  if (!ent) {
    return NULL;
  }

  init = false;
  memset(&st, 0, sizeof(st));

  /* go through all the dictionary pairs */
  while (1) {
    /* parse key */
    com_token = COM_Parse(&data);

    if (com_token[0] == '}') {
      break;
    }

    if (!data) {
      PF_error("ED_ParseEntity: EOF without closing brace");
    }

    Q_strlcpy(keyname, com_token, sizeof(keyname));

    /* parse value */
    com_token = COM_Parse(&data);

    if (!data) {
      PF_error("ED_ParseEntity: EOF without closing brace");
    }

    if (com_token[0] == '}') {
      PF_error("ED_ParseEntity: closing brace without data");
    }

    init = true;

    /* keynames with a leading underscore are
       used for utility comments, and are
       immediately discarded by quake */
    if (keyname[0] == '_') {
      continue;
    }

    ED_ParseField(keyname, com_token, ent);
  }

  if (!init) {
    memset(ent, 0, sizeof(*ent));
  }

  return data;
}

/*
 * Chain together all entities with a matching team field.
 *
 * All but the first will have the FL_TEAMSLAVE flag set.
 * All but the last will have the teamchain field set to the next one
 */
void G_FindTeams(void)
{
  edict_t *e, *e2, *chain;
  int i, j;
  int c, c2;

  c = 0;
  c2 = 0;

  for (i = 1, e = g_edicts + i; i < globals.num_edicts; i++, e++) {
    if (!e->inuse) {
      continue;
    }

    if (!e->team) {
      continue;
    }

    if (e->flags & FL_TEAMSLAVE) {
      continue;
    }

    chain = e;
    e->teammaster = e;
    c++;
    c2++;

    for (j = i + 1, e2 = e + 1; j < globals.num_edicts; j++, e2++) {
      if (!e2->inuse) {
        continue;
      }

      if (!e2->team) {
        continue;
      }

      if (e2->flags & FL_TEAMSLAVE) {
        continue;
      }

      if (!strcmp(e->team, e2->team)) {
        c2++;
        chain->teamchain = e2;
        e2->teammaster = e;
        chain = e2;
        e2->flags |= FL_TEAMSLAVE;
      }
    }
  }

  PF_dprintf("%i teams with %i entities.\n", c, c2);
}

/*
 * Creates a server's entity / program execution context by
 * parsing textual entity definitions out of an ent file.
 */
void SpawnEntities(const char *mapname, char *entities, const char *spawnpoint)
{
  edict_t *ent;
  int inhibit;
  const char *com_token;
  int i;
  float skill_level;

  if (!mapname || !entities || !spawnpoint) {
    return;
  }

  skill_level = floor(skill->value);

  if (skill_level < 0) {
    skill_level = 0;
  }

  if (skill_level > 3) {
    skill_level = 3;
  }

  if (skill->value != skill_level) {
    Cvar_ForceSet("skill", va("%f", skill_level));
  }

  SaveClientData();

  Z_FreeTags(TAG_LEVEL);

  memset(&level, 0, sizeof(level));
  memset(g_edicts, 0, game.maxentities * sizeof(g_edicts[0]));

  Q_strlcpy(level.mapname, mapname, sizeof(level.mapname));
  Q_strlcpy(game.spawnpoint, spawnpoint, sizeof(game.spawnpoint));

  /* set client fields on player ents */
  for (i = 0; i < game.maxclients; i++) {
    g_edicts[i + 1].client = game.clients + i;
  }

  ent = NULL;
  inhibit = 0;

  /* parse ents */
  while (1) {
    /* parse the opening brace */
    com_token = COM_Parse(&entities);

    if (!entities) {
      break;
    }

    if (com_token[0] != '{') {
      PF_error("ED_LoadFromFile: found %s when expecting {", com_token);
    }

    if (!ent) {
      ent = g_edicts;
    } else {
      ent = G_Spawn();
    }

    entities = ED_ParseEdict(entities, ent);

    /* remove things (except the world) from
       different skill levels or deathmatch */
    if (ent != g_edicts) {
      if (deathmatch->value) {
        if (ent->spawnflags & SPAWNFLAG_NOT_DEATHMATCH) {
          G_FreeEdict(ent);
          inhibit++;
          continue;
        }
      } else {
        if (((skill->value == 0) && (ent->spawnflags & SPAWNFLAG_NOT_EASY)) ||
            ((skill->value == 1) && (ent->spawnflags & SPAWNFLAG_NOT_MEDIUM)) ||
            (((skill->value == 2) || (skill->value == 3)) && (ent->spawnflags & SPAWNFLAG_NOT_HARD))) {
          G_FreeEdict(ent);
          inhibit++;
          continue;
        }
      }

      ent->spawnflags &= ~(SPAWNFLAG_NOT_EASY | SPAWNFLAG_NOT_MEDIUM | SPAWNFLAG_NOT_HARD | SPAWNFLAG_NOT_COOP |
                           SPAWNFLAG_NOT_DEATHMATCH);
    }

    ED_CallSpawn(ent);
  }

  PF_dprintf("%i entities inhibited.\n", inhibit);

  G_FindTeams();

  PlayerTrail_Init();
}

/* =================================================================== */

char *singleplayer_statusbar = "yb	-24 "

                               /* health */
                               "xv	0 "
                               "hnum "
                               "xv	50 "
                               "pic 0 "

                               /* ammo */
                               "if 2 "
                               "	xv	100 "
                               "	anum "
                               "	xv	150 "
                               "	pic 2 "
                               "endif "

                               /* armor */
                               "if 4 "
                               "	xv	200 "
                               "	rnum "
                               "	xv	250 "
                               "	pic 4 "
                               "endif "

                               /* selected item */
                               "if 6 "
                               "	xv	296 "
                               "	pic 6 "
                               "endif "

                               "yb	-50 "

                               /* picked up item */
                               "if 7 "
                               "	xv	0 "
                               "	pic 7 "
                               "	xv	26 "
                               "	yb	-42 "
                               "	stat_string 8 "
                               "	yb	-50 "
                               "endif "

                               /* timer */
                               "if 9 "
                               "	xv	262 "
                               "	num	2	10 "
                               "	xv	296 "
                               "	pic	9 "
                               "endif ";

char *deathmatch_statusbar = "yb	-24 "

                             /* health */
                             "xv	0 "
                             "hnum "
                             "xv	50 "
                             "pic 0 "

                             /* ammo */
                             "if 2 "
                             "	xv	100 "
                             "	anum "
                             "	xv	150 "
                             "	pic 2 "
                             "endif "

                             /* armor */
                             "if 4 "
                             "	xv	200 "
                             "	rnum "
                             "	xv	250 "
                             "	pic 4 "
                             "endif "

                             /* selected item */
                             "if 6 "
                             "	xv	296 "
                             "	pic 6 "
                             "endif "

                             "yb	-50 "

                             /* picked up item */
                             "if 7 "
                             "	xv	0 "
                             "	pic 7 "
                             "	xv	26 "
                             "	yb	-42 "
                             "	stat_string 8 "
                             "	yb	-50 "
                             "endif "

                             /* timer */
                             "if 9 "
                             "	xv	246 "
                             "	num	2	10 "
                             "	xv	296 "
                             "	pic	9 "
                             "endif "

                             /*  frags */
                             "xr	-50 "
                             "yt 2 "
                             "num 3 14 "

                             /* spectator */
                             "if 17 "
                             "xv 0 "
                             "yb -58 "
                             "string2 \"SPECTATOR MODE\" "
                             "endif "

                             /* chase camera */
                             "if 16 "
                             "xv 0 "
                             "yb -68 "
                             "string \"Chasing\" "
                             "xv 64 "
                             "stat_string 16 "
                             "endif ";

/*QUAKED worldspawn (0 0 0) ?
 *
 * Only used for the world.
 *  "sky"		environment map name
 *  "skyaxis"	vector axis for rotating sky
 *  "skyrotate"	speed of rotation in degrees/second
 *  "sounds"	music cd track number
 *  "gravity"	800 is default gravity
 *  "message"	text to print at user logon
 */
void SP_worldspawn(edict_t *ent)
{
  if (!ent) {
    return;
  }

  ent->movetype = MOVETYPE_PUSH;
  ent->solid = SOLID_BSP;
  ent->inuse = true;     /* since the world doesn't use G_Spawn() */
  ent->s.modelindex = 1; /* world model is always index 1 */

  /* --------------- */

  /* reserve some spots for dead
     player bodies for coop / deathmatch */
  InitBodyQue();

  /* set configstrings for items */
  SetItemNames();

  if (st.nextmap) {
    strcpy(level.nextmap, st.nextmap);
  }

  /* make some data visible to the server */
  if (ent->message && ent->message[0]) {
    PF_Configstring(CS_NAME, ent->message);
    Q_strlcpy(level.level_name, ent->message, sizeof(level.level_name));
  } else {
    Q_strlcpy(level.level_name, level.mapname, sizeof(level.level_name));
  }

  if (st.sky && st.sky[0]) {
    PF_Configstring(CS_SKY, st.sky);
  } else {
    PF_Configstring(CS_SKY, "unit1_");
  }

  PF_Configstring(CS_SKYROTATE, va("%f", st.skyrotate));

  PF_Configstring(CS_SKYAXIS, va("%f %f %f", st.skyaxis[0], st.skyaxis[1], st.skyaxis[2]));

  PF_Configstring(CS_CDTRACK, va("%i", ent->sounds));

  PF_Configstring(CS_MAXCLIENTS, va("%i", (int) (maxclients->value)));

  /* status bar program */
  if (deathmatch->value) {
    PF_Configstring(CS_STATUSBAR, deathmatch_statusbar);
  } else {
    PF_Configstring(CS_STATUSBAR, singleplayer_statusbar);
  }

  /* --------------- */

  level.pic_health = SV_ImageIndex("icon_health");
  SV_ImageIndex("stat_flash");

  if (!st.gravity) {
    Cvar_Set("sv_gravity", "800");
  } else {
    Cvar_Set("sv_gravity", st.gravity);
  }

  snd_fry = SV_SoundIndex("player/fry.wav"); /* standing in lava / slime */

  PrecacheItem(FindItem("Blaster"));

  SV_SoundIndex("player/lava1.wav");
  SV_SoundIndex("player/lava2.wav");

  SV_SoundIndex("misc/pc_up.wav");
  SV_SoundIndex("misc/talk1.wav");

  SV_SoundIndex("misc/udeath.wav");

  /* gibs */
  SV_SoundIndex("items/respawn1.wav");

  /* sexed sounds */
  SV_SoundIndex("*death1.wav");
  SV_SoundIndex("*death2.wav");
  SV_SoundIndex("*death3.wav");
  SV_SoundIndex("*death4.wav");
  SV_SoundIndex("*fall1.wav");
  SV_SoundIndex("*fall2.wav");
  SV_SoundIndex("*gurp1.wav"); /* drowning damage */
  SV_SoundIndex("*gurp2.wav");
  SV_SoundIndex("*jump1.wav"); /* player jump */
  SV_SoundIndex("*pain25_1.wav");
  SV_SoundIndex("*pain25_2.wav");
  SV_SoundIndex("*pain50_1.wav");
  SV_SoundIndex("*pain50_2.wav");
  SV_SoundIndex("*pain75_1.wav");
  SV_SoundIndex("*pain75_2.wav");
  SV_SoundIndex("*pain100_1.wav");
  SV_SoundIndex("*pain100_2.wav");

  /* sexed models: THIS ORDER MUST MATCH THE DEFINES IN g_local.h
     you can add more, max 19 (pete change)these models are only
     loaded in coop or deathmatch. not singleplayer. */
  if (coop->value || deathmatch->value) {
    SV_ModelIndex("#w_blaster.md2");
    SV_ModelIndex("#w_shotgun.md2");
    SV_ModelIndex("#w_sshotgun.md2");
    SV_ModelIndex("#w_machinegun.md2");
    SV_ModelIndex("#w_chaingun.md2");
    SV_ModelIndex("#a_grenades.md2");
    SV_ModelIndex("#w_glauncher.md2");
    SV_ModelIndex("#w_rlauncher.md2");
    SV_ModelIndex("#w_hyperblaster.md2");
    SV_ModelIndex("#w_railgun.md2");
  }

  /* ------------------- */

  SV_SoundIndex("player/gasp1.wav"); /* gasping for air */
  SV_SoundIndex("player/gasp2.wav"); /* head breaking surface, not gasping */

  SV_SoundIndex("player/watr_in.wav");  /* feet hitting water */
  SV_SoundIndex("player/watr_out.wav"); /* feet leaving water */

  SV_SoundIndex("player/watr_un.wav"); /* head going underwater */

  SV_SoundIndex("player/u_breath1.wav");
  SV_SoundIndex("player/u_breath2.wav");

  SV_SoundIndex("items/pkup.wav");   /* bonus item pickup */
  SV_SoundIndex("world/land.wav");   /* landing thud */
  SV_SoundIndex("misc/h2ohit1.wav"); /* landing splash */

  SV_SoundIndex("items/damage.wav");
  SV_SoundIndex("items/protect.wav");
  SV_SoundIndex("items/protect4.wav");
  SV_SoundIndex("weapons/noammo.wav");

  SV_SoundIndex("infantry/inflies1.wav");

  sm_meat_index = SV_ModelIndex("models/objects/gibs/sm_meat/tris.md2");
  SV_ModelIndex("models/objects/gibs/arm/tris.md2");
  SV_ModelIndex("models/objects/gibs/bone/tris.md2");
  SV_ModelIndex("models/objects/gibs/bone2/tris.md2");
  SV_ModelIndex("models/objects/gibs/chest/tris.md2");
  SV_ModelIndex("models/objects/gibs/skull/tris.md2");
  SV_ModelIndex("models/objects/gibs/head2/tris.md2");

  /* Setup light animation tables. 'a'
     is total darkness, 'z' is doublebright. */

  /* 0 normal */
  PF_Configstring(CS_LIGHTS + 0, "m");

  /* 1 FLICKER (first variety) */
  PF_Configstring(CS_LIGHTS + 1, "mmnmmommommnonmmonqnmmo");

  /* 2 SLOW STRONG PULSE */
  PF_Configstring(CS_LIGHTS + 2, "abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba");

  /* 3 CANDLE (first variety) */
  PF_Configstring(CS_LIGHTS + 3, "mmmmmaaaaammmmmaaaaaabcdefgabcdefg");

  /* 4 FAST STROBE */
  PF_Configstring(CS_LIGHTS + 4, "mamamamamama");

  /* 5 GENTLE PULSE 1 */
  PF_Configstring(CS_LIGHTS + 5, "jklmnopqrstuvwxyzyxwvutsrqponmlkj");

  /* 6 FLICKER (second variety) */
  PF_Configstring(CS_LIGHTS + 6, "nmonqnmomnmomomno");

  /* 7 CANDLE (second variety) */
  PF_Configstring(CS_LIGHTS + 7, "mmmaaaabcdefgmmmmaaaammmaamm");

  /* 8 CANDLE (third variety) */
  PF_Configstring(CS_LIGHTS + 8, "mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa");

  /* 9 SLOW STROBE (fourth variety) */
  PF_Configstring(CS_LIGHTS + 9, "aaaaaaaazzzzzzzz");

  /* 10 FLUORESCENT FLICKER */
  PF_Configstring(CS_LIGHTS + 10, "mmamammmmammamamaaamammma");

  /* 11 SLOW PULSE NOT FADE TO BLACK */
  PF_Configstring(CS_LIGHTS + 11, "abcdefghijklmnopqrrqponmlkjihgfedcba");

  /* styles 32-62 are assigned by the light program for switchable lights */

  /* 63 testing */
  PF_Configstring(CS_LIGHTS + 63, "a");
}
