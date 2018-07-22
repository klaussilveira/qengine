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
 * Jump in into the game.so and support functions.
 *
 * =======================================================================
 */

#include "../server/header/server.h"

field_t fields[] = {{"classname", FOFS(classname), F_LSTRING},
                    {"model", FOFS(model), F_LSTRING},
                    {"spawnflags", FOFS(spawnflags), F_INT},
                    {"speed", FOFS(speed), F_FLOAT},
                    {"accel", FOFS(accel), F_FLOAT},
                    {"decel", FOFS(decel), F_FLOAT},
                    {"target", FOFS(target), F_LSTRING},
                    {"targetname", FOFS(targetname), F_LSTRING},
                    {"pathtarget", FOFS(pathtarget), F_LSTRING},
                    {"deathtarget", FOFS(deathtarget), F_LSTRING},
                    {"killtarget", FOFS(killtarget), F_LSTRING},
                    {"combattarget", FOFS(combattarget), F_LSTRING},
                    {"message", FOFS(message), F_LSTRING},
                    {"team", FOFS(team), F_LSTRING},
                    {"wait", FOFS(wait), F_FLOAT},
                    {"delay", FOFS(delay), F_FLOAT},
                    {"random", FOFS(random), F_FLOAT},
                    {"move_origin", FOFS(move_origin), F_VECTOR},
                    {"move_angles", FOFS(move_angles), F_VECTOR},
                    {"style", FOFS(style), F_INT},
                    {"count", FOFS(count), F_INT},
                    {"health", FOFS(health), F_INT},
                    {"sounds", FOFS(sounds), F_INT},
                    {"light", 0, F_IGNORE},
                    {"dmg", FOFS(dmg), F_INT},
                    {"mass", FOFS(mass), F_INT},
                    {"volume", FOFS(volume), F_FLOAT},
                    {"attenuation", FOFS(attenuation), F_FLOAT},
                    {"map", FOFS(map), F_LSTRING},
                    {"origin", FOFS(s.origin), F_VECTOR},
                    {"angles", FOFS(s.angles), F_VECTOR},
                    {"angle", FOFS(s.angles), F_ANGLEHACK},
                    {"goalentity", FOFS(goalentity), F_EDICT, FFL_NOSPAWN},
                    {"movetarget", FOFS(movetarget), F_EDICT, FFL_NOSPAWN},
                    {"enemy", FOFS(enemy), F_EDICT, FFL_NOSPAWN},
                    {"oldenemy", FOFS(oldenemy), F_EDICT, FFL_NOSPAWN},
                    {"activator", FOFS(activator), F_EDICT, FFL_NOSPAWN},
                    {"groundentity", FOFS(groundentity), F_EDICT, FFL_NOSPAWN},
                    {"teamchain", FOFS(teamchain), F_EDICT, FFL_NOSPAWN},
                    {"teammaster", FOFS(teammaster), F_EDICT, FFL_NOSPAWN},
                    {"owner", FOFS(owner), F_EDICT, FFL_NOSPAWN},
                    {"mynoise", FOFS(mynoise), F_EDICT, FFL_NOSPAWN},
                    {"mynoise2", FOFS(mynoise2), F_EDICT, FFL_NOSPAWN},
                    {"target_ent", FOFS(target_ent), F_EDICT, FFL_NOSPAWN},
                    {"chain", FOFS(chain), F_EDICT, FFL_NOSPAWN},
                    {"lip", STOFS(lip), F_INT, FFL_SPAWNTEMP},
                    {"distance", STOFS(distance), F_INT, FFL_SPAWNTEMP},
                    {"height", STOFS(height), F_INT, FFL_SPAWNTEMP},
                    {"noise", STOFS(noise), F_LSTRING, FFL_SPAWNTEMP},
                    {"pausetime", STOFS(pausetime), F_FLOAT, FFL_SPAWNTEMP},
                    {"item", STOFS(item), F_LSTRING, FFL_SPAWNTEMP},
                    {"item", FOFS(item), F_ITEM},
                    {"gravity", STOFS(gravity), F_LSTRING, FFL_SPAWNTEMP},
                    {"sky", STOFS(sky), F_LSTRING, FFL_SPAWNTEMP},
                    {"skyrotate", STOFS(skyrotate), F_FLOAT, FFL_SPAWNTEMP},
                    {"skyaxis", STOFS(skyaxis), F_VECTOR, FFL_SPAWNTEMP},
                    {"minyaw", STOFS(minyaw), F_FLOAT, FFL_SPAWNTEMP},
                    {"maxyaw", STOFS(maxyaw), F_FLOAT, FFL_SPAWNTEMP},
                    {"minpitch", STOFS(minpitch), F_FLOAT, FFL_SPAWNTEMP},
                    {"maxpitch", STOFS(maxpitch), F_FLOAT, FFL_SPAWNTEMP},
                    {"nextmap", STOFS(nextmap), F_LSTRING, FFL_SPAWNTEMP},
                    {0, 0, 0, 0}};

void InitGame(void)
{
  PF_dprintf("Game is starting up.\n");
  PF_dprintf("Game is %s built on %s.\n", GAMEVERSION, BUILD_DATE);

  gun_x = Cvar_Get("gun_x", "0", 0);
  gun_y = Cvar_Get("gun_y", "0", 0);
  gun_z = Cvar_Get("gun_z", "0", 0);
  sv_rollspeed = Cvar_Get("sv_rollspeed", "200", 0);
  sv_rollangle = Cvar_Get("sv_rollangle", "2", 0);
  sv_maxvelocity = Cvar_Get("sv_maxvelocity", "2000", 0);
  sv_gravity = Cvar_Get("sv_gravity", "800", 0);

  /* noset vars */
  dedicated = Cvar_Get("dedicated", "0", CVAR_NOSET);

  /* latched vars */
  sv_cheats = Cvar_Get("cheats", "0", CVAR_SERVERINFO | CVAR_LATCH);
  Cvar_Get("gamename", GAMEVERSION, CVAR_SERVERINFO | CVAR_LATCH);
  Cvar_Get("gamedate", BUILD_DATE, CVAR_SERVERINFO | CVAR_LATCH);
  maxclients = Cvar_Get("maxclients", "4", CVAR_SERVERINFO | CVAR_LATCH);
  maxspectators = Cvar_Get("maxspectators", "4", CVAR_SERVERINFO);
  deathmatch = Cvar_Get("deathmatch", "0", CVAR_LATCH);
  coop = Cvar_Get("coop", "0", CVAR_LATCH);
  skill = Cvar_Get("skill", "1", CVAR_LATCH);
  maxentities = Cvar_Get("maxentities", "1024", CVAR_LATCH);

  /* change anytime vars */
  dmflags = Cvar_Get("dmflags", "0", CVAR_SERVERINFO);
  fraglimit = Cvar_Get("fraglimit", "0", CVAR_SERVERINFO);
  timelimit = Cvar_Get("timelimit", "0", CVAR_SERVERINFO);
  password = Cvar_Get("password", "", CVAR_USERINFO);
  spectator_password = Cvar_Get("spectator_password", "", CVAR_USERINFO);
  needpass = Cvar_Get("needpass", "0", CVAR_SERVERINFO);
  filterban = Cvar_Get("filterban", "1", 0);
  g_select_empty = Cvar_Get("g_select_empty", "0", CVAR_ARCHIVE);
  run_pitch = Cvar_Get("run_pitch", "0.002", 0);
  run_roll = Cvar_Get("run_roll", "0.005", 0);
  bob_up = Cvar_Get("bob_up", "0.005", 0);
  bob_pitch = Cvar_Get("bob_pitch", "0.002", 0);
  bob_roll = Cvar_Get("bob_roll", "0.002", 0);

  /* flood control */
  flood_msgs = Cvar_Get("flood_msgs", "4", 0);
  flood_persecond = Cvar_Get("flood_persecond", "4", 0);
  flood_waitdelay = Cvar_Get("flood_waitdelay", "10", 0);

  /* dm map list */
  sv_maplist = Cvar_Get("sv_maplist", "", 0);

  /* items */
  InitItems();

  /* initialize all entities for this game */
  game.maxentities = maxentities->value;
  g_edicts = Z_TagMalloc(game.maxentities * sizeof(g_edicts[0]), TAG_GAME);
  globals.edicts = g_edicts;
  globals.max_edicts = game.maxentities;

  /* initialize all clients for this game */
  game.maxclients = maxclients->value;
  game.clients = Z_TagMalloc(game.maxclients * sizeof(game.clients[0]), TAG_GAME);
  globals.num_edicts = game.maxclients + 1;
  globals.edict_size = sizeof(edict_t);
}

void ShutdownGame(void)
{
  PF_dprintf("==== ShutdownGame ====\n");

  Z_FreeTags(TAG_LEVEL);
  Z_FreeTags(TAG_GAME);
}

/* ====================================================================== */

void ClientEndServerFrames(void)
{
  int i;
  edict_t *ent;

  /* calc the player views now that all
     pushing  and damage has been added */
  for (i = 0; i < maxclients->value; i++) {
    ent = g_edicts + 1 + i;

    if (!ent->inuse || !ent->client) {
      continue;
    }

    ClientEndServerFrame(ent);
  }
}

/*
 * Returns the created target changelevel
 */
edict_t *CreateTargetChangeLevel(char *map)
{
  edict_t *ent;

  if (!map) {
    return NULL;
  }

  ent = G_Spawn();
  ent->classname = "target_changelevel";
  Com_sprintf(level.nextmap, sizeof(level.nextmap), "%s", map);
  ent->map = level.nextmap;
  return ent;
}

/*
 * The timelimit or fraglimit has been exceeded
 */
void EndDMLevel(void)
{
  edict_t *ent;
  char *s, *t, *f;
  static const char *seps = " ,\n\r";

  /* stay on same level flag */
  if ((int) dmflags->value & DF_SAME_LEVEL) {
    BeginIntermission(CreateTargetChangeLevel(level.mapname));
    return;
  }

  /* see if it's in the map list */
  if (*sv_maplist->string) {
    s = strdup(sv_maplist->string);
    f = NULL;
    t = strtok(s, seps);

    while (t != NULL) {
      if (Q_stricmp(t, level.mapname) == 0) {
        /* it's in the list, go to the next one */
        t = strtok(NULL, seps);

        if (t == NULL) /* end of list, go to first one */
        {
          if (f == NULL) /* there isn't a first one, same level */
          {
            BeginIntermission(CreateTargetChangeLevel(level.mapname));
          } else {
            BeginIntermission(CreateTargetChangeLevel(f));
          }
        } else {
          BeginIntermission(CreateTargetChangeLevel(t));
        }

        free(s);
        return;
      }

      if (!f) {
        f = t;
      }

      t = strtok(NULL, seps);
    }

    free(s);
  }

  if (level.nextmap[0]) {
    BeginIntermission(CreateTargetChangeLevel(level.nextmap));
  } else {
    ent = G_Find(NULL, FOFS(classname), "target_changelevel");

    if (!ent) {
      BeginIntermission(CreateTargetChangeLevel(level.mapname));
      return;
    }

    BeginIntermission(ent);
  }
}

void CheckNeedPass(void)
{
  int need;

  /* if password or spectator_password has
     changed, update needpass as needed */
  if (password->modified || spectator_password->modified) {
    password->modified = spectator_password->modified = false;

    need = 0;

    if (*password->string && Q_stricmp(password->string, "none")) {
      need |= 1;
    }

    if (*spectator_password->string && Q_stricmp(spectator_password->string, "none")) {
      need |= 2;
    }

    Cvar_Set("needpass", va("%d", need));
  }
}

void CheckDMRules(void)
{
  int i;
  gclient_t *cl;

  if (level.intermissiontime) {
    return;
  }

  if (!deathmatch->value) {
    return;
  }

  if (timelimit->value) {
    if (level.time >= timelimit->value * 60) {
      SV_BroadcastPrintf(PRINT_HIGH, "Timelimit hit.\n");
      EndDMLevel();
      return;
    }
  }

  if (fraglimit->value) {
    for (i = 0; i < maxclients->value; i++) {
      cl = game.clients + i;

      if (!g_edicts[i + 1].inuse) {
        continue;
      }

      if (cl->resp.score >= fraglimit->value) {
        SV_BroadcastPrintf(PRINT_HIGH, "Fraglimit hit.\n");
        EndDMLevel();
        return;
      }
    }
  }
}

void ExitLevel(void)
{
  int i;
  edict_t *ent;
  char command[256];

  Com_sprintf(command, sizeof(command), "gamemap \"%s\"\n", level.changemap);
  Cbuf_AddText(command);
  level.changemap = NULL;
  level.exitintermission = 0;
  level.intermissiontime = 0;
  ClientEndServerFrames();

  /* clear some things before going to next level */
  for (i = 0; i < maxclients->value; i++) {
    ent = g_edicts + 1 + i;

    if (!ent->inuse) {
      continue;
    }

    if (ent->health > ent->client->pers.max_health) {
      ent->health = ent->client->pers.max_health;
    }
  }

  gibsthisframe = 0;
  lastgibframe = 0;
}

/*
 * Advances the world by 0.1 seconds
 */
void G_RunFrame(void)
{
  int i;
  edict_t *ent;

  level.framenum++;
  level.time = level.framenum * FRAMETIME;

  /* choose a client for monsters to target this frame */
  AI_SetSightClient();

  /* exit intermissions */
  if (level.exitintermission) {
    ExitLevel();
    return;
  }

  /* treat each object in turn
     even the world gets a chance
     to think */
  ent = &g_edicts[0];

  for (i = 0; i < globals.num_edicts; i++, ent++) {
    if (!ent->inuse) {
      continue;
    }

    level.current_entity = ent;

    VectorCopy(ent->s.origin, ent->s.old_origin);

    /* if the ground entity moved, make sure we are still on it */
    if ((ent->groundentity) && (ent->groundentity->linkcount != ent->groundentity_linkcount)) {
      ent->groundentity = NULL;

      if (!(ent->flags & (FL_SWIM | FL_FLY)) && (ent->svflags & SVF_MONSTER)) {
        M_CheckGround(ent);
      }
    }

    if ((i > 0) && (i <= maxclients->value)) {
      ClientBeginServerFrame(ent);
      continue;
    }

    G_RunEntity(ent);
  }

  /* see if it is time to end a deathmatch */
  CheckDMRules();

  /* see if needpass needs updated */
  CheckNeedPass();

  /* build the playerstate_t structures for all players */
  ClientEndServerFrames();
}
