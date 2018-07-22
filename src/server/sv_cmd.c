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
 * Server commands received by clients. There are only two ways on which
 * those can be received. Typed via stdin into the server console or via
 * a network / internal communication datagram.
 *
 * =======================================================================
 */

#include "header/server.h"

/*
 * Specify a list of master servers
 */
void SV_SetMaster_f(void)
{
  int i, slot;

  /* only dedicated servers send heartbeats */
  if (!dedicated->value) {
    Com_Printf("Only dedicated servers use masters.\n");
    return;
  }

  /* make sure the server is listed public */
  Cvar_Set("public", "1");

  for (i = 1; i < MAX_MASTERS; i++) {
    memset(&master_adr[i], 0, sizeof(master_adr[i]));
  }

  slot = 1; /* slot 0 will always contain the id master */

  for (i = 1; i < Cmd_Argc(); i++) {
    if (slot == MAX_MASTERS) {
      break;
    }

    if (!NET_StringToAdr(Cmd_Argv(i), &master_adr[i])) {
      Com_Printf("Bad address: %s\n", Cmd_Argv(i));
      continue;
    }

    if (master_adr[slot].port == 0) {
      master_adr[slot].port = BigShort(PORT_MASTER);
    }

    Com_Printf("Master server at %s\n", NET_AdrToString(master_adr[slot]));

    Com_Printf("Sending a ping.\n");

    Netchan_OutOfBandPrint(NS_SERVER, master_adr[slot], "ping");

    slot++;
  }

  svs.last_heartbeat = -9999999;
}

/*
 * Sets sv_client and sv_player to the player with idnum Cmd_Argv(1)
 */
qboolean SV_SetPlayer(void)
{
  client_t *cl;
  int i;
  int idnum;
  char *s;

  if (Cmd_Argc() < 2) {
    return false;
  }

  s = Cmd_Argv(1);

  /* numeric values are just slot numbers */
  if ((s[0] >= '0') && (s[0] <= '9')) {
    idnum = (int) strtol(Cmd_Argv(1), (char **) NULL, 10);

    if ((idnum < 0) || (idnum >= maxclients->value)) {
      Com_Printf("Bad client slot: %i\n", idnum);
      return false;
    }

    sv_client = &svs.clients[idnum];
    sv_player = sv_client->edict;

    if (!sv_client->state) {
      Com_Printf("Client %i is not active\n", idnum);
      return false;
    }

    return true;
  }

  /* check for a name match */
  for (i = 0, cl = svs.clients; i < maxclients->value; i++, cl++) {
    if (!cl->state) {
      continue;
    }

    if (!strcmp(cl->name, s)) {
      sv_client = cl;
      sv_player = sv_client->edict;
      return true;
    }
  }

  Com_Printf("Userid %s is not on the server\n", s);
  return false;
}

/*
 * Saves the state of the map just being exited and goes to a new map.
 */
void SV_GameMap_f(void)
{
  char *map;

  if (Cmd_Argc() != 2) {
    Com_Printf("USAGE: gamemap <map>\n");
    return;
  }

  map = Cmd_Argv(1);

  Com_DPrintf("SV_GameMap(%s)\n", map);

  /* start up the next map */
  SV_Map(false, map, false);

  /* archive server state */
  Q_strlcpy(svs.mapcmd, map, sizeof(svs.mapcmd));
}

/*
 * Goes directly to a given map without any savegame archiving.
 * For development work
 */
void SV_Map_f(void)
{
  char *map;
  char expanded[MAX_QPATH];

  if (Cmd_Argc() != 2) {
    Com_Printf("USAGE: map <mapname>\n");
    return;
  }

  map = Cmd_Argv(1);

  if (!strstr(map, ".") && !strstr(map, "$") && (*map != '*')) {
    Com_sprintf(expanded, sizeof(expanded), "maps/%s.bsp", map);

    if (FS_LoadFile(expanded, NULL) == -1) {
      Com_Printf("Can't find %s\n", expanded);
      return;
    }
  }

  sv.state = ss_dead;
  SV_GameMap_f();
}

/*
 * Kick a user off of the server
 */
void SV_Kick_f(void)
{
  if (!svs.initialized) {
    Com_Printf("No server running.\n");
    return;
  }

  if (Cmd_Argc() != 2) {
    Com_Printf("Usage: kick <userid>\n");
    return;
  }

  if (!SV_SetPlayer()) {
    return;
  }

  if ((sv_client->state == cs_spawned) && *sv_client->name) {
    SV_BroadcastPrintf(PRINT_HIGH, "%s was kicked\n", sv_client->name);
  }

  /* print directly, because the dropped client
     won't get the SV_BroadcastPrintf message */
  SV_ClientPrintf(sv_client, PRINT_HIGH, "You were kicked from the game\n");
  SV_DropClient(sv_client);
  sv_client->lastmessage = svs.realtime; /* min case there is a funny zombie */
}

void SV_Status_f(void)
{
  int i, j, l;
  client_t *cl;
  char *s;
  int ping;

  if (!svs.clients) {
    Com_Printf("No server running.\n");
    return;
  }

  Com_Printf("map              : %s\n", sv.name);

  Com_Printf("num score ping name            lastmsg address               qport \n");
  Com_Printf("--- ----- ---- --------------- ------- --------------------- ------\n");

  for (i = 0, cl = svs.clients; i < maxclients->value; i++, cl++) {
    if (!cl->state) {
      continue;
    }

    Com_Printf("%2i ", i);
    Com_Printf("%5i ", cl->edict->client->ps.stats[STAT_FRAGS]);

    if (cl->state == cs_connected) {
      Com_Printf("CNCT ");
    } else if (cl->state == cs_zombie) {
      Com_Printf("ZMBI ");
    } else {
      ping = cl->ping < 9999 ? cl->ping : 9999;
      Com_Printf("%4i ", ping);
    }

    Com_Printf("%s", cl->name);
    l = 16 - strlen(cl->name);

    for (j = 0; j < l; j++) {
      Com_Printf(" ");
    }

    Com_Printf("%7i ", svs.realtime - cl->lastmessage);

    s = NET_AdrToString(cl->netchan.remote_address);
    Com_Printf("%s", s);
    l = 22 - strlen(s);

    for (j = 0; j < l; j++) {
      Com_Printf(" ");
    }

    Com_Printf("%5i", cl->netchan.qport);

    Com_Printf("\n");
  }

  Com_Printf("\n");
}

void SV_ConSay_f(void)
{
  client_t *client;
  int j;
  char *p;
  char text[1024];

  if (Cmd_Argc() < 2) {
    return;
  }

  if (!svs.initialized) {
    Com_Printf("No server running.\n");
    return;
  }

  strcpy(text, "console: ");
  p = Cmd_Args();

  if (*p == '"') {
    p++;
    p[strlen(p) - 1] = 0;
  }

  strcat(text, p);

  for (j = 0, client = svs.clients; j < maxclients->value; j++, client++) {
    if (client->state != cs_spawned) {
      continue;
    }

    SV_ClientPrintf(client, PRINT_CHAT, "%s\n", text);
  }
}

void SV_Heartbeat_f(void)
{
  svs.last_heartbeat = -9999999;
}

/*
 * Examine or change the serverinfo string
 */
void SV_Serverinfo_f(void)
{
  Com_Printf("Server info settings:\n");
  Info_Print(Cvar_Serverinfo());
}

/*
 * Examine all a users info strings
 */
void SV_DumpUser_f(void)
{
  if (!svs.initialized) {
    Com_Printf("No server running.\n");
    return;
  }

  if (Cmd_Argc() != 2) {
    Com_Printf("Usage: info <userid>\n");
    return;
  }

  if (!SV_SetPlayer()) {
    return;
  }

  Com_Printf("userinfo\n");
  Com_Printf("--------\n");
  Info_Print(sv_client->userinfo);
}

/*
 * Kick everyone off, possibly in preparation for a new game
 */
void SV_KillServer_f(void)
{
  if (!svs.initialized) {
    return;
  }

  SV_Shutdown("Server was killed.\n", false);
  NET_Config(false); /* close network sockets */
}

/*
 * Let the game dll handle a command
 */
void SV_ServerCommand_f(void)
{
  ServerCommand();
}

void SV_InitOperatorCommands(void)
{
  Cmd_AddCommand("heartbeat", SV_Heartbeat_f);
  Cmd_AddCommand("kick", SV_Kick_f);
  Cmd_AddCommand("status", SV_Status_f);
  Cmd_AddCommand("serverinfo", SV_Serverinfo_f);
  Cmd_AddCommand("dumpuser", SV_DumpUser_f);

  Cmd_AddCommand("map", SV_Map_f);
  Cmd_AddCommand("gamemap", SV_GameMap_f);
  Cmd_AddCommand("setmaster", SV_SetMaster_f);

  if (dedicated->value) {
    Cmd_AddCommand("say", SV_ConSay_f);
  }

  Cmd_AddCommand("killserver", SV_KillServer_f);

  Cmd_AddCommand("sv", SV_ServerCommand_f);
}
