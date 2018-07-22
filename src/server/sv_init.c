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
 * Server startup.
 *
 * =======================================================================
 */

#include "header/server.h"

server_static_t svs; /* persistant server info */
server_t sv;         /* local server */

int SV_FindIndex(char *name, int start, int max, qboolean create)
{
  int i;

  if (!name || !name[0]) {
    return 0;
  }

  for (i = 1; i < max && sv.configstrings[start + i][0]; i++) {
    if (!strcmp(sv.configstrings[start + i], name)) {
      return i;
    }
  }

  if (!create) {
    return 0;
  }

  if (i == max) {
    Com_Error(ERR_DROP, "*Index: overflow");
  }

  Q_strlcpy(sv.configstrings[start + i], name, sizeof(sv.configstrings[start + i]));

  if (sv.state != ss_loading) {
    /* send the update to everyone */
    MSG_WriteChar(&sv.multicast, svc_configstring);
    MSG_WriteShort(&sv.multicast, start + i);
    MSG_WriteString(&sv.multicast, name);
    SV_Multicast(vec3_origin, MULTICAST_ALL_R);
  }

  return i;
}

int SV_ModelIndex(char *name)
{
  return SV_FindIndex(name, CS_MODELS, MAX_MODELS, true);
}

int SV_SoundIndex(char *name)
{
  return SV_FindIndex(name, CS_SOUNDS, MAX_SOUNDS, true);
}

int SV_ImageIndex(char *name)
{
  return SV_FindIndex(name, CS_IMAGES, MAX_IMAGES, true);
}

/*
 * Entity baselines are used to compress the update messages
 * to the clients -- only the fields that differ from the
 * baseline will be transmitted
 */
void SV_CreateBaseline(void)
{
  edict_t *svent;
  int entnum;

  for (entnum = 1; entnum < globals.num_edicts; entnum++) {
    svent = EDICT_NUM(entnum);

    if (!svent->inuse) {
      continue;
    }

    if (!svent->s.modelindex && !svent->s.sound && !svent->s.effects) {
      continue;
    }

    svent->s.number = entnum;

    /* take current state as baseline */
    VectorCopy(svent->s.origin, svent->s.old_origin);
    sv.baselines[entnum] = svent->s;
  }
}

/*
 * Change the server to a new map, taking all connected
 * clients along with it.
 */
void SV_SpawnServer(char *server, char *spawnpoint, server_state_t serverstate, qboolean attractloop, qboolean loadgame)
{
  int i;
  unsigned checksum;

  if (attractloop) {
    Cvar_Set("paused", "0");
  }

  Com_Printf("Server init\n");
  Com_DPrintf("SpawnServer: %s\n", server);

  svs.spawncount++; /* any partially connected client will be restarted */
  sv.state = ss_dead;
  Com_SetServerState(sv.state);

  /* wipe the entire per-level structure */
  memset(&sv, 0, sizeof(sv));
  svs.realtime = 0;
  sv.loadgame = loadgame;
  sv.attractloop = attractloop;

  /* save name for levels that don't set message */
  strcpy(sv.configstrings[CS_NAME], server);

  if (Cvar_VariableValue("deathmatch")) {
    sprintf(sv.configstrings[CS_AIRACCEL], "%g", sv_airaccelerate->value);
    pm_airaccelerate = sv_airaccelerate->value;
  } else {
    strcpy(sv.configstrings[CS_AIRACCEL], "0");
    pm_airaccelerate = 0;
  }

  SZ_Init(&sv.multicast, sv.multicast_buf, sizeof(sv.multicast_buf));

  strcpy(sv.name, server);

  /* leave slots at start for clients only */
  for (i = 0; i < maxclients->value; i++) {
    /* needs to reconnect */
    if (svs.clients[i].state > cs_connected) {
      svs.clients[i].state = cs_connected;
    }

    svs.clients[i].lastframe = -1;
  }

  sv.time = 1000;

  strcpy(sv.name, server);
  strcpy(sv.configstrings[CS_NAME], server);

  if (serverstate != ss_game) {
    sv.models[1] = CM_LoadMap("", false, &checksum); /* no real map */
  } else {
    Com_sprintf(sv.configstrings[CS_MODELS + 1], sizeof(sv.configstrings[CS_MODELS + 1]), "maps/%s.bsp", server);
    sv.models[1] = CM_LoadMap(sv.configstrings[CS_MODELS + 1], false, &checksum);
  }

  Com_sprintf(sv.configstrings[CS_MAPCHECKSUM], sizeof(sv.configstrings[CS_MAPCHECKSUM]), "%i", checksum);

  /* clear physics interaction links */
  SV_ClearWorld();

  for (i = 1; i < CM_NumInlineModels(); i++) {
    Com_sprintf(sv.configstrings[CS_MODELS + 1 + i], sizeof(sv.configstrings[CS_MODELS + 1 + i]), "*%i", i);
    sv.models[i + 1] = CM_InlineModel(sv.configstrings[CS_MODELS + 1 + i]);
  }

  /* spawn the rest of the entities on the map */
  sv.state = ss_loading;
  Com_SetServerState(sv.state);

  /* load and spawn all other entities */
  SpawnEntities(sv.name, CM_EntityString(), spawnpoint);

  /* run two frames to allow everything to settle */
  G_RunFrame();
  G_RunFrame();

  /* verify game didn't clobber important stuff */
  if ((int) checksum != (int) strtol(sv.configstrings[CS_MAPCHECKSUM], (char **) NULL, 10)) {
    Com_Error(ERR_DROP, "Game DLL corrupted server configstrings");
  }

  /* all precaches are complete */
  sv.state = serverstate;
  Com_SetServerState(sv.state);

  /* create a baseline for more efficient communications */
  SV_CreateBaseline();

  /* set serverinfo variable */
  Cvar_FullSet("mapname", sv.name, CVAR_SERVERINFO | CVAR_NOSET);
}

/*
 * A brand new game has been started
 */
void SV_InitGame(void)
{
  int i;
  edict_t *ent;
  char idmaster[32];

  if (svs.initialized) {
    /* cause any connected clients to reconnect */
    SV_Shutdown("Server restarted\n", true);
  }

#ifndef DEDICATED_ONLY
  else {
    /* make sure the client is down */
    CL_Drop();
    SCR_BeginLoadingPlaque();
  }
#endif

  /* get any latched variable changes (maxclients, etc) */
  Cvar_GetLatchedVars();

  svs.initialized = true;

  if (Cvar_VariableValue("coop") && Cvar_VariableValue("deathmatch")) {
    Com_Printf("Deathmatch and Coop both set, disabling Coop\n");
    Cvar_FullSet("coop", "0", CVAR_SERVERINFO | CVAR_LATCH);
  }

  /* dedicated servers can't be single player and are usually DM
     so unless they explicity set coop, force it to deathmatch */
  if (dedicated->value) {
    if (!Cvar_VariableValue("coop")) {
      Cvar_FullSet("deathmatch", "1", CVAR_SERVERINFO | CVAR_LATCH);
    }
  }

  /* init clients */
  if (Cvar_VariableValue("deathmatch")) {
    if (maxclients->value <= 1) {
      Cvar_FullSet("maxclients", "8", CVAR_SERVERINFO | CVAR_LATCH);
    } else if (maxclients->value > MAX_CLIENTS) {
      Cvar_FullSet("maxclients", va("%i", MAX_CLIENTS), CVAR_SERVERINFO | CVAR_LATCH);
    }
  } else if (Cvar_VariableValue("coop")) {
    if ((maxclients->value <= 1) || (maxclients->value > 4)) {
      Cvar_FullSet("maxclients", "4", CVAR_SERVERINFO | CVAR_LATCH);
    }
  } else /* non-deathmatch, non-coop is one player */
  {
    Cvar_FullSet("maxclients", "1", CVAR_SERVERINFO | CVAR_LATCH);
  }

  svs.spawncount = randk();
  svs.clients = Z_Malloc(sizeof(client_t) * maxclients->value);
  svs.num_client_entities = maxclients->value * UPDATE_BACKUP * 64;
  svs.client_entities = Z_Malloc(sizeof(entity_state_t) * svs.num_client_entities);

  /* init network stuff */
  NET_Config((maxclients->value > 1));

  /* heartbeats will always be sent to the id master */
  svs.last_heartbeat = -99999; /* send immediately */
  Com_sprintf(idmaster, sizeof(idmaster), "192.246.40.37:%i", PORT_MASTER);
  NET_StringToAdr(idmaster, &master_adr[0]);

  /* init game */
  SV_InitGameProgs();

  for (i = 0; i < maxclients->value; i++) {
    ent = EDICT_NUM(i + 1);
    ent->s.number = i + 1;
    svs.clients[i].edict = ent;
    memset(&svs.clients[i].lastcmd, 0, sizeof(svs.clients[i].lastcmd));
  }
}

void SV_Map(qboolean attractloop, char *levelstring, qboolean loadgame)
{
  char spawnpoint[1];
  spawnpoint[0] = 0;
  sv.loadgame = loadgame;
  sv.attractloop = attractloop;

  if ((sv.state == ss_dead) && !sv.loadgame) {
    SV_InitGame(); /* the game is just starting */
  }

  Cvar_Set("nextserver", "");

#ifndef DEDICATED_ONLY
  SCR_BeginLoadingPlaque(); /* for local system */
#endif
  SV_BroadcastCommand("changing\n");
  SV_SendClientMessages();
  SV_SpawnServer(levelstring, spawnpoint, ss_game, attractloop, loadgame);
  Cbuf_CopyToDefer();

  SV_BroadcastCommand("reconnect\n");
}
