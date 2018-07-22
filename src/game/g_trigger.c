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
 * Trigger.
 *
 * =======================================================================
 */

#include "../server/header/server.h"

#define PUSH_ONCE 1

static int windsound;

void InitTrigger(edict_t *self)
{
  if (!self) {
    return;
  }

  if (!VectorCompare(self->s.angles, vec3_origin)) {
    G_SetMovedir(self->s.angles, self->movedir);
  }

  self->solid = SOLID_TRIGGER;
  self->movetype = MOVETYPE_NONE;
  PF_setmodel(self, self->model);
  self->svflags = SVF_NOCLIENT;
}

/*
 * The wait time has passed, so
 * set back up for another activation
 */
void multi_wait(edict_t *ent)
{
  if (!ent) {
    return;
  }

  ent->nextthink = 0;
}

/*
 * The trigger was just activated
 * ent->activator should be set to
 * the activator so it can be held
 * through a delay so wait for the
 * delay time before firing
 */
void multi_trigger(edict_t *ent)
{
  if (!ent) {
    return;
  }

  if (ent->nextthink) {
    return; /* already been triggered */
  }

  G_UseTargets(ent, ent->activator);

  if (ent->wait > 0) {
    ent->think = multi_wait;
    ent->nextthink = level.time + ent->wait;
  } else {
    /* we can't just remove (self) here,
       because this is a touch function
       called while looping through area
       links... */
    ent->touch = NULL;
    ent->nextthink = level.time + FRAMETIME;
    ent->think = G_FreeEdict;
  }
}

void Use_Multi(edict_t *ent, edict_t *other, edict_t *activator)
{
  if (!ent || !activator) {
    return;
  }

  ent->activator = activator;
  multi_trigger(ent);
}

void Touch_Multi(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
  if (!self || !other) {
    return;
  }

  if (other->client) {
    if (self->spawnflags & 2) {
      return;
    }
  } else if (other->svflags & SVF_MONSTER) {
    if (!(self->spawnflags & 1)) {
      return;
    }
  } else {
    return;
  }

  if (!VectorCompare(self->movedir, vec3_origin)) {
    vec3_t forward;

    AngleVectors(other->s.angles, forward, NULL, NULL);

    if (_DotProduct(forward, self->movedir) < 0) {
      return;
    }
  }

  self->activator = other;
  multi_trigger(self);
}

/*
 * QUAKED trigger_multiple (.5 .5 .5) ? MONSTER NOT_PLAYER TRIGGERED
 * Variable sized repeatable trigger.  Must be targeted at one or more
 * entities. If "delay" is set, the trigger waits some time after
 * activating before firing.
 *
 * "wait" : Seconds between triggerings. (.2 default)
 *
 * sounds
 * 1)	secret
 * 2)	beep beep
 * 3)	large switch
 * 4)
 *
 * set "message" to text string
 */
void trigger_enable(edict_t *self, edict_t *other, edict_t *activator)
{
  if (!self) {
    return;
  }

  self->solid = SOLID_TRIGGER;
  self->use = Use_Multi;
  SV_LinkEdict(self);
}

void SP_trigger_multiple(edict_t *ent)
{
  if (!ent) {
    return;
  }

  if (ent->sounds == 1) {
    ent->noise_index = SV_SoundIndex("misc/secret.wav");
  } else if (ent->sounds == 2) {
    ent->noise_index = SV_SoundIndex("misc/talk.wav");
  } else if (ent->sounds == 3) {
    ent->noise_index = SV_SoundIndex("misc/trigger1.wav");
  }

  if (!ent->wait) {
    ent->wait = 0.2;
  }

  ent->touch = Touch_Multi;
  ent->movetype = MOVETYPE_NONE;
  ent->svflags |= SVF_NOCLIENT;

  if (ent->spawnflags & 4) {
    ent->solid = SOLID_NOT;
    ent->use = trigger_enable;
  } else {
    ent->solid = SOLID_TRIGGER;
    ent->use = Use_Multi;
  }

  if (!VectorCompare(ent->s.angles, vec3_origin)) {
    G_SetMovedir(ent->s.angles, ent->movedir);
  }

  PF_setmodel(ent, ent->model);
  SV_LinkEdict(ent);
}

/*
 * QUAKED trigger_once (.5 .5 .5) ? x x TRIGGERED
 * Triggers once, then removes itself.
 *
 * You must set the key "target" to the name of another
 * object in the level that has a matching "targetname".
 *
 * If TRIGGERED, this trigger must be triggered before it is live.
 *
 * sounds
 *  1) secret
 *  2) beep beep
 *  3) large switch
 *
 * "message" string to be displayed when triggered
 */

void SP_trigger_once(edict_t *ent)
{
  if (!ent) {
    return;
  }

  /* make old maps work because I
     messed up on flag assignments here
     triggered was on bit 1 when it
     should have been on bit 4 */
  if (ent->spawnflags & 1) {
    vec3_t v;

    VectorMA(ent->mins, 0.5, ent->size, v);
    ent->spawnflags &= ~1;
    ent->spawnflags |= 4;
    PF_dprintf("fixed TRIGGERED flag on %s at %s\n", ent->classname, vtos(v));
  }

  ent->wait = -1;
  SP_trigger_multiple(ent);
}

/*
 * QUAKED trigger_relay (.5 .5 .5) (-8 -8 -8) (8 8 8)
 * This fixed size trigger cannot be touched,
 * it can only be fired by other events.
 */
void trigger_relay_use(edict_t *self, edict_t *other, edict_t *activator)
{
  if (!self || !activator) {
    return;
  }

  G_UseTargets(self, activator);
}

void SP_trigger_relay(edict_t *self)
{
  if (!self) {
    return;
  }

  self->use = trigger_relay_use;
}

/*
 * QUAKED trigger_key (.5 .5 .5) (-8 -8 -8) (8 8 8)
 * A relay trigger that only fires it's targets if player
 * has the proper key. Use "item" to specify the required key,
 * for example "key_data_cd"
 */
void trigger_key_use(edict_t *self, edict_t *other, edict_t *activator)
{
  int index;

  if (!self || !activator) {
    return;
  }

  if (!self->item) {
    return;
  }

  if (!activator->client) {
    return;
  }

  index = ITEM_INDEX(self->item);

  if (!activator->client->pers.inventory[index]) {
    if (level.time < self->touch_debounce_time) {
      return;
    }

    self->touch_debounce_time = level.time + 5.0;
    PF_centerprintf(activator, "You need the %s", self->item->pickup_name);
    PF_StartSound(activator, CHAN_AUTO, SV_SoundIndex("misc/keytry.wav"), 1, ATTN_NORM, 0);
    return;
  }

  PF_StartSound(activator, CHAN_AUTO, SV_SoundIndex("misc/keyuse.wav"), 1, ATTN_NORM, 0);

  if (coop->value) {
    int player;
    edict_t *ent;

    for (player = 1; player <= game.maxclients; player++) {
      ent = &g_edicts[player];

      if (!ent->inuse) {
        continue;
      }

      if (!ent->client) {
        continue;
      }

      ent->client->pers.inventory[index] = 0;
    }
  } else {
    activator->client->pers.inventory[index]--;
  }

  G_UseTargets(self, activator);

  self->use = NULL;
}

void SP_trigger_key(edict_t *self)
{
  if (!self) {
    return;
  }

  if (!st.item) {
    PF_dprintf("no key item for trigger_key at %s\n", vtos(self->s.origin));
    return;
  }

  self->item = FindItemByClassname(st.item);

  if (!self->item) {
    PF_dprintf("item %s not found for trigger_key at %s\n", st.item, vtos(self->s.origin));
    return;
  }

  if (!self->target) {
    PF_dprintf("%s at %s has no target\n", self->classname, vtos(self->s.origin));
    return;
  }

  SV_SoundIndex("misc/keytry.wav");
  SV_SoundIndex("misc/keyuse.wav");

  self->use = trigger_key_use;
}

/*
 * QUAKED trigger_counter (.5 .5 .5) ? nomessage
 * Acts as an intermediary for an action that takes multiple inputs.
 *
 * If nomessage is not set, it will print "1 more.. " etc when
 * triggered and "sequence complete" when finished.
 *
 * After the counter has been triggered "count" times (default 2),
 * it will fire all of it's targets and remove itself.
 */
void trigger_counter_use(edict_t *self, edict_t *other, edict_t *activator)
{
  if (!self || !activator) {
    return;
  }

  if (self->count == 0) {
    return;
  }

  self->count--;

  if (self->count) {
    if (!(self->spawnflags & 1)) {
      PF_centerprintf(activator, "%i more to go...", self->count);
      PF_StartSound(activator, CHAN_AUTO, SV_SoundIndex("misc/talk1.wav"), 1, ATTN_NORM, 0);
    }

    return;
  }

  if (!(self->spawnflags & 1)) {
    PF_centerprintf(activator, "Sequence completed!");
    PF_StartSound(activator, CHAN_AUTO, SV_SoundIndex("misc/talk1.wav"), 1, ATTN_NORM, 0);
  }

  self->activator = activator;
  multi_trigger(self);
}

void SP_trigger_counter(edict_t *self)
{
  if (!self) {
    return;
  }

  self->wait = -1;

  if (!self->count) {
    self->count = 2;
  }

  self->use = trigger_counter_use;
}

/*
 * QUAKED trigger_always (.5 .5 .5) (-8 -8 -8) (8 8 8)
 * This trigger will always fire. It is activated by the world.
 */
void SP_trigger_always(edict_t *ent)
{
  if (!ent) {
    return;
  }

  /* we must have some delay to make
     sure our use targets are present */
  if (ent->delay < 0.2) {
    ent->delay = 0.2;
  }

  G_UseTargets(ent, ent);
}

void trigger_push_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
  if (!self || !other) {
    return;
  }

  if (strcmp(other->classname, "grenade") == 0) {
    VectorScale(self->movedir, self->speed * 10, other->velocity);
  } else if (other->health > 0) {
    VectorScale(self->movedir, self->speed * 10, other->velocity);

    if (other->client) {
      /* don't take falling damage
         immediately from this */
      VectorCopy(other->velocity, other->client->oldvelocity);

      if (other->fly_sound_debounce_time < level.time) {
        other->fly_sound_debounce_time = level.time + 1.5;
        PF_StartSound(other, CHAN_AUTO, windsound, 1, ATTN_NORM, 0);
      }
    }
  }

  if (self->spawnflags & PUSH_ONCE) {
    G_FreeEdict(self);
  }
}

/*
 * QUAKED trigger_push (.5 .5 .5) ? PUSH_ONCE
 * Pushes the player
 *
 * "speed"		defaults to 1000
 */
void SP_trigger_push(edict_t *self)
{
  if (!self) {
    return;
  }

  InitTrigger(self);
  windsound = SV_SoundIndex("misc/windfly.wav");
  self->touch = trigger_push_touch;

  if (!self->speed) {
    self->speed = 1000;
  }

  SV_LinkEdict(self);
}

/*
 * QUAKED trigger_hurt (.5 .5 .5) ? START_OFF TOGGLE SILENT NO_PROTECTION SLOW
 * Any entity that touches this will be hurt.
 *
 * It does dmg points of damage each server frame
 *
 * SILENT			supresses playing the sound
 * SLOW				changes the damage rate to once per second
 * NO_PROTECTION	*nothing* stops the damage
 *
 * "dmg"			default 5 (whole numbers only)
 */
void hurt_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
  int dflags;

  if (!self || !other) {
    return;
  }

  if (!other->takedamage) {
    return;
  }

  if (self->timestamp > level.time) {
    return;
  }

  if (self->spawnflags & 16) {
    self->timestamp = level.time + 1;
  } else {
    self->timestamp = level.time + FRAMETIME;
  }

  if (!(self->spawnflags & 4)) {
    if ((level.framenum % 10) == 0) {
      PF_StartSound(other, CHAN_AUTO, self->noise_index, 1, ATTN_NORM, 0);
    }
  }

  if (self->spawnflags & 8) {
    dflags = DAMAGE_NO_PROTECTION;
  } else {
    dflags = 0;
  }

  T_Damage(other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, self->dmg, dflags,
           MOD_TRIGGER_HURT);
}

void hurt_use(edict_t *self, edict_t *other, edict_t *activator)
{
  if (!self) {
    return;
  }

  if (self->solid == SOLID_NOT) {
    int i, num;
    edict_t *touch[MAX_EDICTS], *hurtme;

    self->solid = SOLID_TRIGGER;
    num = SV_AreaEdicts(self->absmin, self->absmax, touch, MAX_EDICTS, AREA_SOLID);

    /* Check for idle monsters in
       trigger hurt */
    for (i = 0; i < num; i++) {
      hurtme = touch[i];
      hurt_touch(self, hurtme, NULL, NULL);
    }
  } else {
    self->solid = SOLID_NOT;
  }

  SV_LinkEdict(self);

  if (!(self->spawnflags & 2)) {
    self->use = NULL;
  }
}

void SP_trigger_hurt(edict_t *self)
{
  if (!self) {
    return;
  }

  InitTrigger(self);

  self->noise_index = SV_SoundIndex("world/electro.wav");
  self->touch = hurt_touch;

  if (!self->dmg) {
    self->dmg = 5;
  }

  if (self->spawnflags & 1) {
    self->solid = SOLID_NOT;
  } else {
    self->solid = SOLID_TRIGGER;
  }

  if (self->spawnflags & 2) {
    self->use = hurt_use;
  }

  SV_LinkEdict(self);
}

/*
 * QUAKED trigger_gravity (.5 .5 .5) ?
 * Changes the touching entites gravity to
 * the value of "gravity".  1.0 is standard
 * gravity for the level.
 */

void trigger_gravity_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
  if (!self || !other) {
    return;
  }

  other->gravity = self->gravity;
}

void SP_trigger_gravity(edict_t *self)
{
  if (!self) {
    return;
  }

  if (st.gravity == 0) {
    PF_dprintf("trigger_gravity without gravity set at %s\n", vtos(self->s.origin));
    G_FreeEdict(self);
    return;
  }

  InitTrigger(self);
  self->gravity = (int) strtol(st.gravity, (char **) NULL, 10);
  self->touch = trigger_gravity_touch;
}

/*
 * QUAKED trigger_monsterjump (.5 .5 .5) ?
 * Walking monsters that touch this will jump in the direction of the trigger's
 * angle
 *
 * "speed"  default to 200, the speed thrown forward
 * "height" default to 200, the speed thrown upwards
 */

void trigger_monsterjump_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
  if (!self || !other) {
    return;
  }

  if (other->flags & (FL_FLY | FL_SWIM)) {
    return;
  }

  if (other->svflags & SVF_DEADMONSTER) {
    return;
  }

  if (!(other->svflags & SVF_MONSTER)) {
    return;
  }

  /* set XY even if not on ground, so the jump will clear lips */
  other->velocity[0] = self->movedir[0] * self->speed;
  other->velocity[1] = self->movedir[1] * self->speed;

  if (!other->groundentity) {
    return;
  }

  other->groundentity = NULL;
  other->velocity[2] = self->movedir[2];
}

void SP_trigger_monsterjump(edict_t *self)
{
  if (!self) {
    return;
  }

  if (!self->speed) {
    self->speed = 200;
  }

  if (!st.height) {
    st.height = 200;
  }

  if (self->s.angles[YAW] == 0) {
    self->s.angles[YAW] = 360;
  }

  InitTrigger(self);
  self->touch = trigger_monsterjump_touch;
  self->movedir[2] = st.height;
}
