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
 * Item handling and item definitions.
 *
 * =======================================================================
 */

#include "../server/header/server.h"

#define HEALTH_IGNORE_MAX 1
#define HEALTH_TIMED 2

qboolean Pickup_Weapon(edict_t *ent, edict_t *other);

void Use_Weapon(edict_t *ent, gitem_t *inv);

void Drop_Weapon(edict_t *ent, gitem_t *inv);

void Weapon_Blaster(edict_t *ent);

void Weapon_Shotgun(edict_t *ent);

void Weapon_SuperShotgun(edict_t *ent);

void Weapon_Machinegun(edict_t *ent);

void Weapon_Chaingun(edict_t *ent);

void Weapon_HyperBlaster(edict_t *ent);

void Weapon_RocketLauncher(edict_t *ent);

void Weapon_Grenade(edict_t *ent);

void Weapon_GrenadeLauncher(edict_t *ent);

void Weapon_Railgun(edict_t *ent);

gitem_armor_t jacketarmor_info = {25, 50, .30, .00, ARMOR_JACKET};
gitem_armor_t combatarmor_info = {50, 100, .60, .30, ARMOR_COMBAT};
gitem_armor_t bodyarmor_info = {100, 200, .80, .60, ARMOR_BODY};

int jacket_armor_index;
int combat_armor_index;
int body_armor_index;

/* ====================================================================== */

gitem_t *GetItemByIndex(int index)
{
  if ((index == 0) || (index >= game.num_items)) {
    return NULL;
  }

  return &itemlist[index];
}

gitem_t *FindItemByClassname(char *classname)
{
  int i;
  gitem_t *it;

  if (!classname) {
    return NULL;
  }

  it = itemlist;

  for (i = 0; i < game.num_items; i++, it++) {
    if (!it->classname) {
      continue;
    }

    if (!Q_stricmp(it->classname, classname)) {
      return it;
    }
  }

  return NULL;
}

gitem_t *FindItem(char *pickup_name)
{
  int i;
  gitem_t *it;

  if (!pickup_name) {
    return NULL;
  }

  it = itemlist;

  for (i = 0; i < game.num_items; i++, it++) {
    if (!it->pickup_name) {
      continue;
    }

    if (!Q_stricmp(it->pickup_name, pickup_name)) {
      return it;
    }
  }

  return NULL;
}

/* ====================================================================== */

void DoRespawn(edict_t *ent)
{
  if (!ent) {
    return;
  }

  if (ent->team) {
    edict_t *master;
    int count;
    int choice;

    master = ent->teammaster;

    for (count = 0, ent = master; ent; ent = ent->chain, count++) {
    }

    choice = count ? randk() % count : 0;

    for (count = 0, ent = master; count < choice; ent = ent->chain, count++) {
    }
  }

  ent->svflags &= ~SVF_NOCLIENT;
  ent->solid = SOLID_TRIGGER;
  SV_LinkEdict(ent);

  /* send an effect */
  ent->s.event = EV_ITEM_RESPAWN;
}

void SetRespawn(edict_t *ent, float delay)
{
  if (!ent) {
    return;
  }

  ent->flags |= FL_RESPAWN;
  ent->svflags |= SVF_NOCLIENT;
  ent->solid = SOLID_NOT;
  ent->nextthink = level.time + delay;
  ent->think = DoRespawn;
  SV_LinkEdict(ent);
}

/* ====================================================================== */

qboolean Pickup_Powerup(edict_t *ent, edict_t *other)
{
  int quantity;

  if (!ent || !other) {
    return false;
  }

  quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];

  if (((skill->value == 1) && (quantity >= 2)) || ((skill->value >= 2) && (quantity >= 1))) {
    return false;
  }

  if ((coop->value) && (ent->item->flags & IT_STAY_COOP) && (quantity > 0)) {
    return false;
  }

  other->client->pers.inventory[ITEM_INDEX(ent->item)]++;

  if (deathmatch->value) {
    if (!(ent->spawnflags & DROPPED_ITEM)) {
      SetRespawn(ent, ent->item->quantity);
    }

    if (((int) dmflags->value & DF_INSTANT_ITEMS)) {
      ent->item->use(other, ent->item);
    }
  }

  return true;
}

void Drop_General(edict_t *ent, gitem_t *item)
{
  Drop_Item(ent, item);
  ent->client->pers.inventory[ITEM_INDEX(item)]--;
  ValidateSelectedItem(ent);
}

/* ====================================================================== */

qboolean Pickup_Key(edict_t *ent, edict_t *other)
{
  if (!ent || !other) {
    return false;
  }

  if (coop->value) {
    if (other->client->pers.inventory[ITEM_INDEX(ent->item)]) {
      return false;
    }

    other->client->pers.inventory[ITEM_INDEX(ent->item)] = 1;

    return true;
  }

  other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
  return true;
}

/* ====================================================================== */

qboolean Add_Ammo(edict_t *ent, gitem_t *item, int count)
{
  int index;
  int max;

  if (!ent || !item) {
    return false;
  }

  if (!ent->client) {
    return false;
  }

  if (item->tag == AMMO_BULLETS) {
    max = ent->client->pers.max_bullets;
  } else if (item->tag == AMMO_SHELLS) {
    max = ent->client->pers.max_shells;
  } else if (item->tag == AMMO_ROCKETS) {
    max = ent->client->pers.max_rockets;
  } else if (item->tag == AMMO_GRENADES) {
    max = ent->client->pers.max_grenades;
  } else if (item->tag == AMMO_CELLS) {
    max = ent->client->pers.max_cells;
  } else if (item->tag == AMMO_SLUGS) {
    max = ent->client->pers.max_slugs;
  } else {
    return false;
  }

  index = ITEM_INDEX(item);

  if (ent->client->pers.inventory[index] == max) {
    return false;
  }

  ent->client->pers.inventory[index] += count;

  if (ent->client->pers.inventory[index] > max) {
    ent->client->pers.inventory[index] = max;
  }

  return true;
}

qboolean Pickup_Ammo(edict_t *ent, edict_t *other)
{
  int oldcount;
  int count;
  qboolean weapon;

  if (!ent || !other) {
    return false;
  }

  weapon = (ent->item->flags & IT_WEAPON);

  if ((weapon) && ((int) dmflags->value & DF_INFINITE_AMMO)) {
    count = 1000;
  } else if (ent->count) {
    count = ent->count;
  } else {
    count = ent->item->quantity;
  }

  oldcount = other->client->pers.inventory[ITEM_INDEX(ent->item)];

  if (!Add_Ammo(other, ent->item, count)) {
    return false;
  }

  if (weapon && !oldcount) {
    if ((other->client->pers.weapon != ent->item) &&
        (!deathmatch->value || (other->client->pers.weapon == FindItem("blaster")))) {
      other->client->newweapon = ent->item;
    }
  }

  if (!(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)) && (deathmatch->value)) {
    SetRespawn(ent, 30);
  }

  return true;
}

void Drop_Ammo(edict_t *ent, gitem_t *item)
{
  edict_t *dropped;
  int index;

  if (!ent || !item) {
    return;
  }

  index = ITEM_INDEX(item);
  dropped = Drop_Item(ent, item);

  if (ent->client->pers.inventory[index] >= item->quantity) {
    dropped->count = item->quantity;
  } else {
    dropped->count = ent->client->pers.inventory[index];
  }

  if (ent->client->pers.weapon && (ent->client->pers.weapon->tag == AMMO_GRENADES) && (item->tag == AMMO_GRENADES) &&
      (ent->client->pers.inventory[index] - dropped->count <= 0)) {
    PF_cprintf(ent, PRINT_HIGH, "Can't drop current weapon\n");
    G_FreeEdict(dropped);
    return;
  }

  ent->client->pers.inventory[index] -= dropped->count;
  ValidateSelectedItem(ent);
}

/* ====================================================================== */

void MegaHealth_think(edict_t *self)
{
  if (!self) {
    return;
  }

  if (self->owner->health > self->owner->max_health) {
    self->nextthink = level.time + 1;
    self->owner->health -= 1;
    return;
  }

  if (!(self->spawnflags & DROPPED_ITEM) && (deathmatch->value)) {
    SetRespawn(self, 20);
  } else {
    G_FreeEdict(self);
  }
}

qboolean Pickup_Health(edict_t *ent, edict_t *other)
{
  if (!ent || !other) {
    return false;
  }

  if (!(ent->style & HEALTH_IGNORE_MAX)) {
    if (other->health >= other->max_health) {
      return false;
    }
  }

  other->health += ent->count;

  if (!(ent->style & HEALTH_IGNORE_MAX)) {
    if (other->health > other->max_health) {
      other->health = other->max_health;
    }
  }

  if (ent->style & HEALTH_TIMED) {
    ent->think = MegaHealth_think;
    ent->nextthink = level.time + 5;
    ent->owner = other;
    ent->flags |= FL_RESPAWN;
    ent->svflags |= SVF_NOCLIENT;
    ent->solid = SOLID_NOT;
  } else {
    if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value)) {
      SetRespawn(ent, 30);
    }
  }

  return true;
}

/* ====================================================================== */

int ArmorIndex(edict_t *ent)
{
  if (!ent) {
    return 0;
  }

  if (!ent->client) {
    return 0;
  }

  if (ent->client->pers.inventory[jacket_armor_index] > 0) {
    return jacket_armor_index;
  }

  if (ent->client->pers.inventory[combat_armor_index] > 0) {
    return combat_armor_index;
  }

  if (ent->client->pers.inventory[body_armor_index] > 0) {
    return body_armor_index;
  }

  return 0;
}

qboolean Pickup_Armor(edict_t *ent, edict_t *other)
{
  int old_armor_index;
  gitem_armor_t *oldinfo;
  gitem_armor_t *newinfo;
  int newcount;
  float salvage;
  int salvagecount;

  if (!ent || !other) {
    return false;
  }

  /* get info on new armor */
  newinfo = (gitem_armor_t *) ent->item->info;

  old_armor_index = ArmorIndex(other);

  /* handle armor shards specially */
  if (ent->item->tag == ARMOR_SHARD) {
    if (!old_armor_index) {
      other->client->pers.inventory[jacket_armor_index] = 2;
    } else {
      other->client->pers.inventory[old_armor_index] += 2;
    }
  } else if (!old_armor_index) /* if player has no armor, just use it */
  {
    other->client->pers.inventory[ITEM_INDEX(ent->item)] = newinfo->base_count;
  } else /* use the better armor */
  {
    /* get info on old armor */
    if (old_armor_index == jacket_armor_index) {
      oldinfo = &jacketarmor_info;
    } else if (old_armor_index == combat_armor_index) {
      oldinfo = &combatarmor_info;
    } else {
      oldinfo = &bodyarmor_info;
    }

    if (newinfo->normal_protection > oldinfo->normal_protection) {
      /* calc new armor values */
      salvage = oldinfo->normal_protection / newinfo->normal_protection;
      salvagecount = salvage * other->client->pers.inventory[old_armor_index];
      newcount = newinfo->base_count + salvagecount;

      if (newcount > newinfo->max_count) {
        newcount = newinfo->max_count;
      }

      /* zero count of old armor so it goes away */
      other->client->pers.inventory[old_armor_index] = 0;

      /* change armor to new item with computed value */
      other->client->pers.inventory[ITEM_INDEX(ent->item)] = newcount;
    } else {
      /* calc new armor values */
      salvage = newinfo->normal_protection / oldinfo->normal_protection;
      salvagecount = salvage * newinfo->base_count;
      newcount = other->client->pers.inventory[old_armor_index] + salvagecount;

      if (newcount > oldinfo->max_count) {
        newcount = oldinfo->max_count;
      }

      /* if we're already maxed out then we don't need the new armor */
      if (other->client->pers.inventory[old_armor_index] >= newcount) {
        return false;
      }

      /* update current armor value */
      other->client->pers.inventory[old_armor_index] = newcount;
    }
  }

  if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value)) {
    SetRespawn(ent, 20);
  }

  return true;
}

/* ====================================================================== */

void Touch_Item(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
  qboolean taken;

  if (!ent || !other) {
    return;
  }

  if (!other->client) {
    return;
  }

  if (other->health < 1) {
    return; /* dead people can't pickup */
  }

  if (!ent->item->pickup) {
    return; /* not a grabbable item? */
  }

  taken = ent->item->pickup(ent, other);

  if (taken) {
    /* flash the screen */
    other->client->bonus_alpha = 0.25;

    /* show icon and name on status bar */
    other->client->ps.stats[STAT_PICKUP_ICON] = SV_ImageIndex(ent->item->icon);
    other->client->ps.stats[STAT_PICKUP_STRING] = CS_ITEMS + ITEM_INDEX(ent->item);
    other->client->pickup_msg_time = level.time + 3.0;

    /* change selected item */
    if (ent->item->use) {
      other->client->pers.selected_item = other->client->ps.stats[STAT_SELECTED_ITEM] = ITEM_INDEX(ent->item);
    }

    if (ent->item->pickup == Pickup_Health) {
      if (ent->count == 2) {
        PF_StartSound(other, CHAN_ITEM, SV_SoundIndex("items/s_health.wav"), 1, ATTN_NORM, 0);
      } else if (ent->count == 10) {
        PF_StartSound(other, CHAN_ITEM, SV_SoundIndex("items/n_health.wav"), 1, ATTN_NORM, 0);
      } else if (ent->count == 25) {
        PF_StartSound(other, CHAN_ITEM, SV_SoundIndex("items/l_health.wav"), 1, ATTN_NORM, 0);
      } else /* (ent->count == 100) */
      {
        PF_StartSound(other, CHAN_ITEM, SV_SoundIndex("items/m_health.wav"), 1, ATTN_NORM, 0);
      }
    } else if (ent->item->pickup_sound) {
      PF_StartSound(other, CHAN_ITEM, SV_SoundIndex(ent->item->pickup_sound), 1, ATTN_NORM, 0);
    }
  }

  if (!(ent->spawnflags & ITEM_TARGETS_USED)) {
    G_UseTargets(ent, other);
    ent->spawnflags |= ITEM_TARGETS_USED;
  }

  if (!taken) {
    return;
  }

  if (!((coop->value) && (ent->item->flags & IT_STAY_COOP)) ||
      (ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM))) {
    if (ent->flags & FL_RESPAWN) {
      ent->flags &= ~FL_RESPAWN;
    } else {
      G_FreeEdict(ent);
    }
  }
}

/* ====================================================================== */

void drop_temp_touch(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
  if (!ent || !other) {
    return;
  }

  if (other == ent->owner) {
    return;
  }

  /* plane and surf are unused in Touch_Item
     but since the function is part of the
     game <-> client interface dropping
     them is too much pain. */
  Touch_Item(ent, other, plane, surf);
}

void drop_make_touchable(edict_t *ent)
{
  if (!ent) {
    return;
  }

  ent->touch = Touch_Item;

  if (deathmatch->value) {
    ent->nextthink = level.time + 29;
    ent->think = G_FreeEdict;
  }
}

edict_t *Drop_Item(edict_t *ent, gitem_t *item)
{
  edict_t *dropped;
  vec3_t forward, right;
  vec3_t offset;

  if (!ent || !item) {
    return NULL;
  }

  dropped = G_Spawn();

  dropped->classname = item->classname;
  dropped->item = item;
  dropped->spawnflags = DROPPED_ITEM;
  dropped->s.effects = item->world_model_flags;
  dropped->s.renderfx = RF_GLOW;

  if (randk() > 0.5) {
    dropped->s.angles[1] += randk() * 45;
  } else {
    dropped->s.angles[1] -= randk() * 45;
  }

  VectorSet(dropped->mins, -16, -16, -16);
  VectorSet(dropped->maxs, 16, 16, 16);
  PF_setmodel(dropped, dropped->item->world_model);
  dropped->solid = SOLID_TRIGGER;
  dropped->movetype = MOVETYPE_TOSS;
  dropped->touch = drop_temp_touch;
  dropped->owner = ent;

  if (ent->client) {
    trace_t trace;

    AngleVectors(ent->client->v_angle, forward, right, NULL);
    VectorSet(offset, 24, 0, -16);
    G_ProjectSource(ent->s.origin, offset, forward, right, dropped->s.origin);
    trace = SV_Trace(ent->s.origin, dropped->mins, dropped->maxs, dropped->s.origin, ent, CONTENTS_SOLID);
    VectorCopy(trace.endpos, dropped->s.origin);
  } else {
    AngleVectors(ent->s.angles, forward, right, NULL);
    VectorCopy(ent->s.origin, dropped->s.origin);
  }

  VectorScale(forward, 100, dropped->velocity);
  dropped->velocity[2] = 300;

  dropped->think = drop_make_touchable;
  dropped->nextthink = level.time + 1;

  SV_LinkEdict(dropped);

  return dropped;
}

void Use_Item(edict_t *ent, edict_t *other, edict_t *activator)
{
  if (!ent) {
    return;
  }

  ent->svflags &= ~SVF_NOCLIENT;
  ent->use = NULL;

  if (ent->spawnflags & ITEM_NO_TOUCH) {
    ent->solid = SOLID_BBOX;
    ent->touch = NULL;
  } else {
    ent->solid = SOLID_TRIGGER;
    ent->touch = Touch_Item;
  }

  SV_LinkEdict(ent);
}

/* ====================================================================== */

void droptofloor(edict_t *ent)
{
  trace_t tr;
  vec3_t dest;
  float *v;

  if (!ent) {
    return;
  }

  v = tv(-15, -15, -15);
  VectorCopy(v, ent->mins);
  v = tv(15, 15, 15);
  VectorCopy(v, ent->maxs);

  if (ent->model) {
    PF_setmodel(ent, ent->model);
  } else {
    PF_setmodel(ent, ent->item->world_model);
  }

  ent->solid = SOLID_TRIGGER;
  ent->movetype = MOVETYPE_TOSS;
  ent->touch = Touch_Item;

  v = tv(0, 0, -128);
  VectorAdd(ent->s.origin, v, dest);

  tr = SV_Trace(ent->s.origin, ent->mins, ent->maxs, dest, ent, MASK_SOLID);

  if (tr.startsolid) {
    PF_dprintf("droptofloor: %s startsolid at %s\n", ent->classname, vtos(ent->s.origin));
    G_FreeEdict(ent);
    return;
  }

  VectorCopy(tr.endpos, ent->s.origin);

  if (ent->team) {
    ent->flags &= ~FL_TEAMSLAVE;
    ent->chain = ent->teamchain;
    ent->teamchain = NULL;

    ent->svflags |= SVF_NOCLIENT;
    ent->solid = SOLID_NOT;

    if (ent == ent->teammaster) {
      ent->nextthink = level.time + FRAMETIME;
      ent->think = DoRespawn;
    }
  }

  if (ent->spawnflags & ITEM_NO_TOUCH) {
    ent->solid = SOLID_BBOX;
    ent->touch = NULL;
    ent->s.effects &= ~EF_ROTATE;
    ent->s.renderfx &= ~RF_GLOW;
  }

  if (ent->spawnflags & ITEM_TRIGGER_SPAWN) {
    ent->svflags |= SVF_NOCLIENT;
    ent->solid = SOLID_NOT;
    ent->use = Use_Item;
  }

  SV_LinkEdict(ent);
}

/*
 * Precaches all data needed for a given item.
 * This will be called for each item spawned in a level,
 * and for each item in each client's inventory.
 */
void PrecacheItem(gitem_t *it)
{
  char *s, *start;
  char data[MAX_QPATH];
  int len;
  gitem_t *ammo;

  if (!it) {
    return;
  }

  if (it->pickup_sound) {
    SV_SoundIndex(it->pickup_sound);
  }

  if (it->world_model) {
    SV_ModelIndex(it->world_model);
  }

  if (it->view_model) {
    SV_ModelIndex(it->view_model);
  }

  if (it->icon) {
    SV_ImageIndex(it->icon);
  }

  /* parse everything for its ammo */
  if (it->ammo && it->ammo[0]) {
    ammo = FindItem(it->ammo);

    if (ammo != it) {
      PrecacheItem(ammo);
    }
  }

  /* parse the space seperated precache string for other items */
  s = it->precaches;

  if (!s || !s[0]) {
    return;
  }

  while (*s) {
    start = s;

    while (*s && *s != ' ') {
      s++;
    }

    len = s - start;

    if ((len >= MAX_QPATH) || (len < 5)) {
      PF_error("PrecacheItem: %s has bad precache string", it->classname);
    }

    memcpy(data, start, len);
    data[len] = 0;

    if (*s) {
      s++;
    }

    /* determine type based on extension */
    if (!strcmp(data + len - 3, "md2")) {
      SV_ModelIndex(data);
    } else if (!strcmp(data + len - 3, "sp2")) {
      SV_ModelIndex(data);
    } else if (!strcmp(data + len - 3, "wav")) {
      SV_SoundIndex(data);
    }

    if (!strcmp(data + len - 3, "pcx")) {
      SV_ImageIndex(data);
    }
  }
}

/*
 * ============
 * Sets the clipping size and
 * plants the object on the floor.
 *
 * Items can't be immediately dropped
 * to floor, because they might be on
 * an entity that hasn't spawned yet.
 * ============
 */
void SpawnItem(edict_t *ent, gitem_t *item)
{
  if (!ent || !item) {
    return;
  }

  PrecacheItem(item);

  /* some items will be prevented in deathmatch */
  if (deathmatch->value) {
    if ((int) dmflags->value & DF_NO_ARMOR) {
      if ((item->pickup == Pickup_Armor)) {
        G_FreeEdict(ent);
        return;
      }
    }

    if ((int) dmflags->value & DF_NO_ITEMS) {
      if (item->pickup == Pickup_Powerup) {
        G_FreeEdict(ent);
        return;
      }
    }

    if ((int) dmflags->value & DF_NO_HEALTH) {
      if ((item->pickup == Pickup_Health)) {
        G_FreeEdict(ent);
        return;
      }
    }
  }

  /* don't let them drop items that stay in a coop game */
  if ((coop->value) && (item->flags & IT_STAY_COOP)) {
    item->drop = NULL;
  }

  ent->item = item;
  ent->nextthink = level.time + 2 * FRAMETIME; /* items start after other solids */
  ent->think = droptofloor;
  ent->s.effects = item->world_model_flags;
  ent->s.renderfx = RF_GLOW;

  if (ent->model) {
    SV_ModelIndex(ent->model);
  }
}

/* ====================================================================== */

gitem_t itemlist[] = {
    {NULL}, /* leave index 0 alone */

    /* QUAKED item_armor_body (.3 .3 1) (-16 -16 -16) (16 16 16) */
    {"item_armor_body", Pickup_Armor, NULL, NULL, NULL, "misc/ar1_pkup.wav", "models/items/armor/body/tris.md2",
     EF_ROTATE, NULL, "i_bodyarmor", "Body Armor", 3, 0, NULL, IT_ARMOR, 0, &bodyarmor_info, ARMOR_BODY, ""},

    /* QUAKED item_armor_combat (.3 .3 1) (-16 -16 -16) (16 16 16) */
    {"item_armor_combat", Pickup_Armor, NULL, NULL, NULL, "misc/ar1_pkup.wav", "models/items/armor/combat/tris.md2",
     EF_ROTATE, NULL, "i_combatarmor", "Combat Armor", 3, 0, NULL, IT_ARMOR, 0, &combatarmor_info, ARMOR_COMBAT, ""},

    /* QUAKED item_armor_jacket (.3 .3 1) (-16 -16 -16) (16 16 16) */
    {"item_armor_jacket", Pickup_Armor, NULL, NULL, NULL, "misc/ar1_pkup.wav", "models/items/armor/jacket/tris.md2",
     EF_ROTATE, NULL, "i_jacketarmor", "Jacket Armor", 3, 0, NULL, IT_ARMOR, 0, &jacketarmor_info, ARMOR_JACKET, ""},

    /* QUAKED item_armor_shard (.3 .3 1) (-16 -16 -16) (16 16 16) */
    {"item_armor_shard", Pickup_Armor, NULL, NULL, NULL, "misc/ar2_pkup.wav", "models/items/armor/shard/tris.md2",
     EF_ROTATE, NULL, "i_jacketarmor", "Armor Shard", 3, 0, NULL, IT_ARMOR, 0, NULL, ARMOR_SHARD, ""},

    /* weapon_blaster (.3 .3 1) (-16 -16 -16) (16 16 16)
       always owned, never in the world */
    {"weapon_blaster", NULL, Use_Weapon, NULL, Weapon_Blaster, "misc/w_pkup.wav", NULL, 0,
     "models/weapons/v_blast/tris.md2", "w_blaster", "Blaster", 0, 0, NULL, IT_WEAPON | IT_STAY_COOP, WEAP_BLASTER,
     NULL, 0, "weapons/blastf1a.wav misc/lasfly.wav"},

    /* QUAKED weapon_shotgun (.3 .3 1) (-16 -16 -16) (16 16 16) */
    {"weapon_shotgun", Pickup_Weapon, Use_Weapon, Drop_Weapon, Weapon_Shotgun, "misc/w_pkup.wav",
     "models/weapons/g_shotg/tris.md2", EF_ROTATE, "models/weapons/v_shotg/tris.md2", "w_shotgun", "Shotgun", 0, 1,
     "Shells", IT_WEAPON | IT_STAY_COOP, WEAP_SHOTGUN, NULL, 0, "weapons/shotgf1b.wav weapons/shotgr1b.wav"},

    /* QUAKED weapon_supershotgun (.3 .3 1) (-16 -16 -16) (16 16 16) */
    {"weapon_supershotgun", Pickup_Weapon, Use_Weapon, Drop_Weapon, Weapon_SuperShotgun, "misc/w_pkup.wav",
     "models/weapons/g_shotg2/tris.md2", EF_ROTATE, "models/weapons/v_shotg2/tris.md2", "w_sshotgun", "Super Shotgun",
     0, 2, "Shells", IT_WEAPON | IT_STAY_COOP, WEAP_SUPERSHOTGUN, NULL, 0, "weapons/sshotf1b.wav"},

    /* QUAKED weapon_machinegun (.3 .3 1) (-16 -16 -16) (16 16 16) */
    {"weapon_machinegun", Pickup_Weapon, Use_Weapon, Drop_Weapon, Weapon_Machinegun, "misc/w_pkup.wav",
     "models/weapons/g_machn/tris.md2", EF_ROTATE, "models/weapons/v_machn/tris.md2", "w_machinegun", "Machinegun", 0,
     1, "Bullets", IT_WEAPON | IT_STAY_COOP, WEAP_MACHINEGUN, NULL, 0,
     "weapons/machgf1b.wav weapons/machgf2b.wav weapons/machgf3b.wav "
     "weapons/machgf4b.wav weapons/machgf5b.wav"},

    /* QUAKED weapon_chaingun (.3 .3 1) (-16 -16 -16) (16 16 16) */
    {"weapon_chaingun", Pickup_Weapon, Use_Weapon, Drop_Weapon, Weapon_Chaingun, "misc/w_pkup.wav",
     "models/weapons/g_chain/tris.md2", EF_ROTATE, "models/weapons/v_chain/tris.md2", "w_chaingun", "Chaingun", 0, 1,
     "Bullets", IT_WEAPON | IT_STAY_COOP, WEAP_CHAINGUN, NULL, 0,
     "weapons/chngnu1a.wav weapons/chngnl1a.wav weapons/machgf3b.wav` "
     "weapons/chngnd1a.wav"},

    /* QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16) */
    {"ammo_grenades", Pickup_Ammo, Use_Weapon, Drop_Ammo, Weapon_Grenade, "misc/am_pkup.wav",
     "models/items/ammo/grenades/medium/tris.md2", 0, "models/weapons/v_handgr/tris.md2", "a_grenades", "Grenades", 3,
     5, "grenades", IT_AMMO | IT_WEAPON, WEAP_GRENADES, NULL, AMMO_GRENADES,
     "weapons/hgrent1a.wav weapons/hgrena1b.wav weapons/hgrenc1b.wav "
     "weapons/hgrenb1a.wav weapons/hgrenb2a.wav "},

    /* QUAKED weapon_grenadelauncher (.3 .3 1) (-16 -16 -16) (16 16 16) */
    {"weapon_grenadelauncher", Pickup_Weapon, Use_Weapon, Drop_Weapon, Weapon_GrenadeLauncher, "misc/w_pkup.wav",
     "models/weapons/g_launch/tris.md2", EF_ROTATE, "models/weapons/v_launch/tris.md2", "w_glauncher",
     "Grenade Launcher", 0, 1, "Grenades", IT_WEAPON | IT_STAY_COOP, WEAP_GRENADELAUNCHER, NULL, 0,
     "models/objects/grenade/tris.md2 weapons/grenlf1a.wav "
     "weapons/grenlr1b.wav weapons/grenlb1b.wav"},

    /* QUAKED weapon_rocketlauncher (.3 .3 1) (-16 -16 -16) (16 16 16) */
    {"weapon_rocketlauncher", Pickup_Weapon, Use_Weapon, Drop_Weapon, Weapon_RocketLauncher, "misc/w_pkup.wav",
     "models/weapons/g_rocket/tris.md2", EF_ROTATE, "models/weapons/v_rocket/tris.md2", "w_rlauncher",
     "Rocket Launcher", 0, 1, "Rockets", IT_WEAPON | IT_STAY_COOP, WEAP_ROCKETLAUNCHER, NULL, 0,
     "models/objects/rocket/tris.md2 weapons/rockfly.wav weapons/rocklf1a.wav "
     "weapons/rocklr1b.wav models/objects/debris2/tris.md2"},

    /* QUAKED weapon_hyperblaster (.3 .3 1) (-16 -16 -16) (16 16 16) */
    {"weapon_hyperblaster", Pickup_Weapon, Use_Weapon, Drop_Weapon, Weapon_HyperBlaster, "misc/w_pkup.wav",
     "models/weapons/g_hyperb/tris.md2", EF_ROTATE, "models/weapons/v_hyperb/tris.md2", "w_hyperblaster",
     "HyperBlaster", 0, 1, "Cells", IT_WEAPON | IT_STAY_COOP, WEAP_HYPERBLASTER, NULL, 0,
     "weapons/hyprbu1a.wav weapons/hyprbl1a.wav weapons/hyprbf1a.wav "
     "weapons/hyprbd1a.wav misc/lasfly.wav"},

    /* QUAKED weapon_railgun (.3 .3 1) (-16 -16 -16) (16 16 16) */
    {"weapon_railgun", Pickup_Weapon, Use_Weapon, Drop_Weapon, Weapon_Railgun, "misc/w_pkup.wav",
     "models/weapons/g_rail/tris.md2", EF_ROTATE, "models/weapons/v_rail/tris.md2", "w_railgun", "Railgun", 0, 1,
     "Slugs", IT_WEAPON | IT_STAY_COOP, WEAP_RAILGUN, NULL, 0, "weapons/rg_hum.wav"},

    /* QUAKED ammo_shells (.3 .3 1) (-16 -16 -16) (16 16 16) */
    {"ammo_shells", Pickup_Ammo, NULL, Drop_Ammo, NULL, "misc/am_pkup.wav", "models/items/ammo/shells/medium/tris.md2",
     0, NULL, "a_shells", "Shells", 3, 10, NULL, IT_AMMO, 0, NULL, AMMO_SHELLS, ""},

    /* QUAKED ammo_bullets (.3 .3 1) (-16 -16 -16) (16 16 16) */
    {"ammo_bullets", Pickup_Ammo, NULL, Drop_Ammo, NULL, "misc/am_pkup.wav",
     "models/items/ammo/bullets/medium/tris.md2", 0, NULL, "a_bullets", "Bullets", 3, 50, NULL, IT_AMMO, 0, NULL,
     AMMO_BULLETS, ""},

    /* QUAKED ammo_cells (.3 .3 1) (-16 -16 -16) (16 16 16) */
    {"ammo_cells", Pickup_Ammo, NULL, Drop_Ammo, NULL, "misc/am_pkup.wav", "models/items/ammo/cells/medium/tris.md2", 0,
     NULL, "a_cells", "Cells", 3, 50, NULL, IT_AMMO, 0, NULL, AMMO_CELLS, ""},

    /* QUAKED ammo_rockets (.3 .3 1) (-16 -16 -16) (16 16 16) */
    {"ammo_rockets", Pickup_Ammo, NULL, Drop_Ammo, NULL, "misc/am_pkup.wav",
     "models/items/ammo/rockets/medium/tris.md2", 0, NULL, "a_rockets", "Rockets", 3, 5, NULL, IT_AMMO, 0, NULL,
     AMMO_ROCKETS, ""},

    /* QUAKED ammo_slugs (.3 .3 1) (-16 -16 -16) (16 16 16) */
    {"ammo_slugs", Pickup_Ammo, NULL, Drop_Ammo, NULL, "misc/am_pkup.wav", "models/items/ammo/slugs/medium/tris.md2", 0,
     NULL, "a_slugs", "Slugs", 3, 10, NULL, IT_AMMO, 0, NULL, AMMO_SLUGS, ""},

    /* QUAKED key_blue_key (0 .5 .8) (-16 -16 -16) (16 16 16)
       normal door key - blue */
    {"key_blue_key", Pickup_Key, NULL, Drop_General, NULL, "items/pkup.wav", "models/items/keys/key/tris.md2",
     EF_ROTATE, NULL, "k_bluekey", "Blue Key", 2, 0, NULL, IT_STAY_COOP | IT_KEY, 0, NULL, 0, ""},

    /* QUAKED key_red_key (0 .5 .8) (-16 -16 -16) (16 16 16)
       normal door key - red */
    {"key_red_key", Pickup_Key, NULL, Drop_General, NULL, "items/pkup.wav", "models/items/keys/red_key/tris.md2",
     EF_ROTATE, NULL, "k_redkey", "Red Key", 2, 0, NULL, IT_STAY_COOP | IT_KEY, 0, NULL, 0, ""},

    {NULL, Pickup_Health, NULL, NULL, NULL, "items/pkup.wav", NULL, 0, NULL, "i_health", "Health", 3, 0, NULL, 0, 0,
     NULL, 0,
     "items/s_health.wav items/n_health.wav items/l_health.wav "
     "items/m_health.wav"},

    /* end of list marker */
    {NULL}};

/*
 * QUAKED item_health (.3 .3 1) (-16 -16 -16) (16 16 16)
 */
void SP_item_health(edict_t *self)
{
  if (!self) {
    return;
  }

  if (deathmatch->value && ((int) dmflags->value & DF_NO_HEALTH)) {
    G_FreeEdict(self);
    return;
  }

  self->model = "models/items/healing/medium/tris.md2";
  self->count = 10;
  SpawnItem(self, FindItem("Health"));
  SV_SoundIndex("items/n_health.wav");
}

/*
 * QUAKED item_health_small (.3 .3 1) (-16 -16 -16) (16 16 16)
 */
void SP_item_health_small(edict_t *self)
{
  if (!self) {
    return;
  }

  if (deathmatch->value && ((int) dmflags->value & DF_NO_HEALTH)) {
    G_FreeEdict(self);
    return;
  }

  self->model = "models/items/healing/stimpack/tris.md2";
  self->count = 2;
  SpawnItem(self, FindItem("Health"));
  self->style = HEALTH_IGNORE_MAX;
  SV_SoundIndex("items/s_health.wav");
}

/*
 * QUAKED item_health_large (.3 .3 1) (-16 -16 -16) (16 16 16)
 */
void SP_item_health_large(edict_t *self)
{
  if (!self) {
    return;
  }

  if (deathmatch->value && ((int) dmflags->value & DF_NO_HEALTH)) {
    G_FreeEdict(self);
    return;
  }

  self->model = "models/items/healing/large/tris.md2";
  self->count = 25;
  SpawnItem(self, FindItem("Health"));
  SV_SoundIndex("items/l_health.wav");
}

/*
 * QUAKED item_health_mega (.3 .3 1) (-16 -16 -16) (16 16 16)
 */
void SP_item_health_mega(edict_t *self)
{
  if (!self) {
    return;
  }

  if (deathmatch->value && ((int) dmflags->value & DF_NO_HEALTH)) {
    G_FreeEdict(self);
    return;
  }

  self->model = "models/items/mega_h/tris.md2";
  self->count = 100;
  SpawnItem(self, FindItem("Health"));
  SV_SoundIndex("items/m_health.wav");
  self->style = HEALTH_IGNORE_MAX | HEALTH_TIMED;
}

void InitItems(void)
{
  game.num_items = sizeof(itemlist) / sizeof(itemlist[0]) - 1;
}

/*
 * Called by worldspawn
 */
void SetItemNames(void)
{
  int i;
  gitem_t *it;

  for (i = 0; i < game.num_items; i++) {
    it = &itemlist[i];
    PF_Configstring(CS_ITEMS + i, it->pickup_name);
  }

  jacket_armor_index = ITEM_INDEX(FindItem("Jacket Armor"));
  combat_armor_index = ITEM_INDEX(FindItem("Combat Armor"));
  body_armor_index = ITEM_INDEX(FindItem("Body Armor"));
}
