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
 * Miscellaneos entities, functs and functions.
 *
 * =======================================================================
 */

#include "../server/header/server.h"

int gibsthisframe;
int lastgibframe;

void Use_Areaportal(edict_t *ent, edict_t *other, edict_t *activator)
{
  if (!ent) {
    return;
  }

  ent->count ^= 1; /* toggle state */
  CM_SetAreaPortalState(ent->style, ent->count);
}

/*
 * QUAKED func_areaportal (0 0 0) ?
 *
 * This is a non-visible object that divides the world into
 * areas that are seperated when this portal is not activated.
 * Usually enclosed in the middle of a door.
 */
void SP_func_areaportal(edict_t *ent)
{
  if (!ent) {
    return;
  }

  ent->use = Use_Areaportal;
  ent->count = 0; /* always start closed; */
}

/* ===================================================== */

void VelocityForDamage(int damage, vec3_t v)
{
  v[0] = 100.0 * crandom();
  v[1] = 100.0 * crandom();
  v[2] = 200.0 + 100.0 * random();

  if (damage < 50) {
    VectorScale(v, 0.7, v);
  } else {
    VectorScale(v, 1.2, v);
  }
}

void ClipGibVelocity(edict_t *ent)
{
  if (!ent) {
    return;
  }

  if (ent->velocity[0] < -300) {
    ent->velocity[0] = -300;
  } else if (ent->velocity[0] > 300) {
    ent->velocity[0] = 300;
  }

  if (ent->velocity[1] < -300) {
    ent->velocity[1] = -300;
  } else if (ent->velocity[1] > 300) {
    ent->velocity[1] = 300;
  }

  if (ent->velocity[2] < 200) {
    ent->velocity[2] = 200; /* always some upwards */
  } else if (ent->velocity[2] > 500) {
    ent->velocity[2] = 500;
  }
}

/* ===================================================== */

void gib_think(edict_t *self)
{
  if (!self) {
    return;
  }

  self->s.frame++;
  self->nextthink = level.time + FRAMETIME;

  if (self->s.frame == 10) {
    self->think = G_FreeEdict;
    self->nextthink = level.time + 8 + random() * 10;
  }
}

void gib_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
  if (!self || !plane) {
    return;
  }

  vec3_t normal_angles, right;

  if (!self->groundentity) {
    return;
  }

  self->touch = NULL;

  if (plane) {
    PF_StartSound(self, CHAN_VOICE, SV_SoundIndex("misc/fhit3.wav"), 1, ATTN_NORM, 0);

    vectoangles(plane->normal, normal_angles);
    AngleVectors(normal_angles, NULL, right, NULL);
    vectoangles(right, self->s.angles);

    if (self->s.modelindex == sm_meat_index) {
      self->s.frame++;
      self->think = gib_think;
      self->nextthink = level.time + FRAMETIME;
    }
  }
}

void gib_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
  if (!self) {
    return;
  }

  G_FreeEdict(self);
}

void ThrowGib(edict_t *self, char *gibname, int damage, int type)
{
  edict_t *gib;
  vec3_t vd;
  vec3_t origin;
  vec3_t size;
  float vscale;

  if (!self || !gibname) {
    return;
  }

  if (level.framenum > lastgibframe) {
    gibsthisframe = 0;
    lastgibframe = level.framenum;
  }

  gibsthisframe++;

  if (gibsthisframe > 20) {
    return;
  }

  gib = G_Spawn();

  VectorScale(self->size, 0.5, size);
  VectorAdd(self->absmin, size, origin);
  gib->s.origin[0] = origin[0] + crandom() * size[0];
  gib->s.origin[1] = origin[1] + crandom() * size[1];
  gib->s.origin[2] = origin[2] + crandom() * size[2];

  PF_setmodel(gib, gibname);
  gib->solid = SOLID_BBOX;
  gib->s.effects |= EF_GIB;
  gib->flags |= FL_NO_KNOCKBACK;
  gib->takedamage = DAMAGE_YES;
  gib->die = gib_die;

  if (type == GIB_ORGANIC) {
    gib->movetype = MOVETYPE_TOSS;
    gib->touch = gib_touch;
    vscale = 0.5;
  } else {
    gib->movetype = MOVETYPE_BOUNCE;
    vscale = 1.0;
  }

  VelocityForDamage(damage, vd);
  VectorMA(self->velocity, vscale, vd, gib->velocity);
  ClipGibVelocity(gib);
  gib->avelocity[0] = random() * 600;
  gib->avelocity[1] = random() * 600;
  gib->avelocity[2] = random() * 600;

  gib->think = G_FreeEdict;
  gib->nextthink = level.time + 10 + random() * 10;

  SV_LinkEdict(gib);
}

void ThrowHead(edict_t *self, char *gibname, int damage, int type)
{
  vec3_t vd;
  float vscale;

  if (!self || !gibname) {
    return;
  }

  self->s.skinnum = 0;
  self->s.frame = 0;
  VectorClear(self->mins);
  VectorClear(self->maxs);

  self->s.modelindex2 = 0;
  PF_setmodel(self, gibname);
  self->solid = SOLID_BBOX;
  self->s.effects |= EF_GIB;
  self->s.effects &= ~EF_FLIES;
  self->s.sound = 0;
  self->flags |= FL_NO_KNOCKBACK;
  self->svflags &= ~SVF_MONSTER;
  self->takedamage = DAMAGE_YES;
  self->targetname = NULL;
  self->die = gib_die;

  if (type == GIB_ORGANIC) {
    self->movetype = MOVETYPE_TOSS;
    self->touch = gib_touch;
    vscale = 0.5;
  } else {
    self->movetype = MOVETYPE_BOUNCE;
    vscale = 1.0;
  }

  VelocityForDamage(damage, vd);
  VectorMA(self->velocity, vscale, vd, self->velocity);
  ClipGibVelocity(self);

  self->avelocity[YAW] = crandom() * 600;

  self->think = G_FreeEdict;
  self->nextthink = level.time + 10 + random() * 10;

  SV_LinkEdict(self);
}

void ThrowClientHead(edict_t *self, int damage)
{
  vec3_t vd;
  char *gibname;

  if (!self) {
    return;
  }

  if (randk() & 1) {
    gibname = "models/objects/gibs/head2/tris.md2";
    self->s.skinnum = 1; /* second skin is player */
  } else {
    gibname = "models/objects/gibs/skull/tris.md2";
    self->s.skinnum = 0;
  }

  self->s.origin[2] += 32;
  self->s.frame = 0;
  PF_setmodel(self, gibname);
  VectorSet(self->mins, -16, -16, 0);
  VectorSet(self->maxs, 16, 16, 16);

  self->takedamage = DAMAGE_NO;
  self->solid = SOLID_BBOX;
  self->s.effects = EF_GIB;
  self->s.sound = 0;
  self->flags |= FL_NO_KNOCKBACK;

  self->movetype = MOVETYPE_BOUNCE;
  VelocityForDamage(damage, vd);
  VectorAdd(self->velocity, vd, self->velocity);

  if (self->client) /* bodies in the queue don't have a client anymore */
  {
    self->client->anim_priority = ANIM_DEATH;
    self->client->anim_end = self->s.frame;
  } else {
    self->think = NULL;
    self->nextthink = 0;
  }

  SV_LinkEdict(self);
}

/* ===================================================== */

void debris_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
  if (!self) {
    return;
  }

  G_FreeEdict(self);
}

void ThrowDebris(edict_t *self, char *modelname, float speed, vec3_t origin)
{
  edict_t *chunk;
  vec3_t v;

  if (!self || !modelname) {
    return;
  }

  if (level.framenum > lastgibframe) {
    gibsthisframe = 0;
    lastgibframe = level.framenum;
  }

  gibsthisframe++;

  if (gibsthisframe > 20) {
    return;
  }

  chunk = G_Spawn();
  VectorCopy(origin, chunk->s.origin);
  PF_setmodel(chunk, modelname);
  v[0] = 100 * crandom();
  v[1] = 100 * crandom();
  v[2] = 100 + 100 * crandom();
  VectorMA(self->velocity, speed, v, chunk->velocity);
  chunk->movetype = MOVETYPE_BOUNCE;
  chunk->solid = SOLID_NOT;
  chunk->avelocity[0] = random() * 600;
  chunk->avelocity[1] = random() * 600;
  chunk->avelocity[2] = random() * 600;
  chunk->think = G_FreeEdict;
  chunk->nextthink = level.time + 5 + random() * 5;
  chunk->s.frame = 0;
  chunk->flags = 0;
  chunk->classname = "debris";
  chunk->takedamage = DAMAGE_YES;
  chunk->die = debris_die;
  SV_LinkEdict(chunk);
}

void BecomeExplosion1(edict_t *self)
{
  if (!self) {
    return;
  }

  PF_WriteByte(svc_temp_entity);
  PF_WriteByte(TE_EXPLOSION1);
  PF_WritePos(self->s.origin);
  SV_Multicast(self->s.origin, MULTICAST_PVS);

  G_FreeEdict(self);
}

void BecomeExplosion2(edict_t *self)
{
  if (!self) {
    return;
  }

  PF_WriteByte(svc_temp_entity);
  PF_WriteByte(TE_EXPLOSION2);
  PF_WritePos(self->s.origin);
  SV_Multicast(self->s.origin, MULTICAST_PVS);

  G_FreeEdict(self);
}
/* ===================================================== */

/*
 * QUAKED path_corner (.5 .3 0) (-8 -8 -8) (8 8 8) TELEPORT
 * Target: next path corner
 * Pathtarget: gets used when an entity that has
 *             this path_corner targeted touches it
 */
void path_corner_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
  vec3_t v;
  edict_t *next;

  if (!self || !other) {
    return;
  }

  if (other->movetarget != self) {
    return;
  }

  if (other->enemy) {
    return;
  }

  if (self->pathtarget) {
    char *savetarget;

    savetarget = self->target;
    self->target = self->pathtarget;
    G_UseTargets(self, other);
    self->target = savetarget;
  }

  if (self->target) {
    next = G_PickTarget(self->target);
  } else {
    next = NULL;
  }

  if ((next) && (next->spawnflags & 1)) {
    VectorCopy(next->s.origin, v);
    v[2] += next->mins[2];
    v[2] -= other->mins[2];
    VectorCopy(v, other->s.origin);
    next = G_PickTarget(next->target);
    other->s.event = EV_OTHER_TELEPORT;
  }

  other->goalentity = other->movetarget = next;

  if (self->wait) {
    other->monsterinfo.pausetime = level.time + self->wait;
    other->monsterinfo.stand(other);
    return;
  }

  if (!other->movetarget) {
    other->monsterinfo.pausetime = level.time + 100000000;
    other->monsterinfo.stand(other);
  } else {
    VectorSubtract(other->goalentity->s.origin, other->s.origin, v);
    other->ideal_yaw = vectoyaw(v);
  }
}

void SP_path_corner(edict_t *self)
{
  if (!self) {
    return;
  }

  if (!self->targetname) {
    PF_dprintf("path_corner with no targetname at %s\n", vtos(self->s.origin));
    G_FreeEdict(self);
    return;
  }

  self->solid = SOLID_TRIGGER;
  self->touch = path_corner_touch;
  VectorSet(self->mins, -8, -8, -8);
  VectorSet(self->maxs, 8, 8, 8);
  self->svflags |= SVF_NOCLIENT;
  SV_LinkEdict(self);
}

/* ===================================================== */

/*
 * QUAKED point_combat (0.5 0.3 0) (-8 -8 -8) (8 8 8) Hold
 *
 * Makes this the target of a monster and it will head here
 * when first activated before going after the activator.  If
 * hold is selected, it will stay here.
 */
void point_combat_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
  edict_t *activator;

  if (!self || !other) {
    return;
  }

  if (other->movetarget != self) {
    return;
  }

  if (self->target) {
    other->target = self->target;
    other->goalentity = other->movetarget = G_PickTarget(other->target);

    if (!other->goalentity) {
      PF_dprintf("%s at %s target %s does not exist\n", self->classname, vtos(self->s.origin), self->target);
      other->movetarget = self;
    }

    self->target = NULL;
  } else if ((self->spawnflags & 1) && !(other->flags & (FL_SWIM | FL_FLY))) {
    other->monsterinfo.pausetime = level.time + 100000000;
    other->monsterinfo.aiflags |= AI_STAND_GROUND;
    other->monsterinfo.stand(other);
  }

  if (other->movetarget == self) {
    other->target = NULL;
    other->movetarget = NULL;
    other->goalentity = other->enemy;
    other->monsterinfo.aiflags &= ~AI_COMBAT_POINT;
  }

  if (self->pathtarget) {
    char *savetarget;

    savetarget = self->target;
    self->target = self->pathtarget;

    if (other->enemy && other->enemy->client) {
      activator = other->enemy;
    } else if (other->oldenemy && other->oldenemy->client) {
      activator = other->oldenemy;
    } else if (other->activator && other->activator->client) {
      activator = other->activator;
    } else {
      activator = other;
    }

    G_UseTargets(self, activator);
    self->target = savetarget;
  }
}

void SP_point_combat(edict_t *self)
{
  if (!self) {
    return;
  }

  if (deathmatch->value) {
    G_FreeEdict(self);
    return;
  }

  self->solid = SOLID_TRIGGER;
  self->touch = point_combat_touch;
  VectorSet(self->mins, -8, -8, -16);
  VectorSet(self->maxs, 8, 8, 16);
  self->svflags = SVF_NOCLIENT;
  SV_LinkEdict(self);
}

/* ===================================================== */

/*
 * QUAKED info_null (0 0.5 0) (-4 -4 -4) (4 4 4)
 * Used as a positional target for spotlights, etc.
 */
void SP_info_null(edict_t *self)
{
  if (!self) {
    return;
  }

  G_FreeEdict(self);
}

/*
 * QUAKED info_notnull (0 0.5 0) (-4 -4 -4) (4 4 4)
 * Used as a positional target for lightning.
 */
void SP_info_notnull(edict_t *self)
{
  if (!self) {
    return;
  }

  VectorCopy(self->s.origin, self->absmin);
  VectorCopy(self->s.origin, self->absmax);
}

#define START_OFF 1

/*
 * QUAKED light (0 1 0) (-8 -8 -8) (8 8 8) START_OFF
 * Non-displayed light.
 * Default light value is 300.
 * Default style is 0.
 * If targeted, will toggle between on and off.
 * Default _cone value is 10 (used to set size of light for spotlights)
 */
void light_use(edict_t *self, edict_t *other, edict_t *activator)
{
  if (!self) {
    return;
  }

  if (self->spawnflags & START_OFF) {
    PF_Configstring(CS_LIGHTS + self->style, "m");
    self->spawnflags &= ~START_OFF;
  } else {
    PF_Configstring(CS_LIGHTS + self->style, "a");
    self->spawnflags |= START_OFF;
  }
}

void SP_light(edict_t *self)
{
  if (!self) {
    return;
  }

  /* no targeted lights in deathmatch, because they cause global messages */
  if (!self->targetname || deathmatch->value) {
    G_FreeEdict(self);
    return;
  }

  if (self->style >= 32) {
    self->use = light_use;

    if (self->spawnflags & START_OFF) {
      PF_Configstring(CS_LIGHTS + self->style, "a");
    } else {
      PF_Configstring(CS_LIGHTS + self->style, "m");
    }
  }
}

/* ===================================================== */

/*
 * QUAKED func_wall (0 .5 .8) ? TRIGGER_SPAWN TOGGLE START_ON ANIMATED
 * ANIMATED_FAST This is just a solid wall if not inhibited
 *
 * TRIGGER_SPAWN	the wall will not be present until triggered
 *                  it will then blink in to existance; it will
 *                  kill anything that was in it's way
 *
 * TOGGLE			only valid for TRIGGER_SPAWN walls
 *                  this allows the wall to be turned on and off
 *
 * START_ON		only valid for TRIGGER_SPAWN walls
 *              the wall will initially be present
 */
void func_wall_use(edict_t *self, edict_t *other, edict_t *activator)
{
  if (!self) {
    return;
  }

  if (self->solid == SOLID_NOT) {
    self->solid = SOLID_BSP;
    self->svflags &= ~SVF_NOCLIENT;
    KillBox(self);
  } else {
    self->solid = SOLID_NOT;
    self->svflags |= SVF_NOCLIENT;
  }

  SV_LinkEdict(self);

  if (!(self->spawnflags & 2)) {
    self->use = NULL;
  }
}

void SP_func_wall(edict_t *self)
{
  if (!self) {
    return;
  }

  self->movetype = MOVETYPE_PUSH;
  PF_setmodel(self, self->model);

  if (self->spawnflags & 8) {
    self->s.effects |= EF_ANIM_ALL;
  }

  if (self->spawnflags & 16) {
    self->s.effects |= EF_ANIM_ALLFAST;
  }

  /* just a wall */
  if ((self->spawnflags & 7) == 0) {
    self->solid = SOLID_BSP;
    SV_LinkEdict(self);
    return;
  }

  /* it must be TRIGGER_SPAWN */
  if (!(self->spawnflags & 1)) {
    self->spawnflags |= 1;
  }

  /* yell if the spawnflags are odd */
  if (self->spawnflags & 4) {
    if (!(self->spawnflags & 2)) {
      PF_dprintf("func_wall START_ON without TOGGLE\n");
      self->spawnflags |= 2;
    }
  }

  self->use = func_wall_use;

  if (self->spawnflags & 4) {
    self->solid = SOLID_BSP;
  } else {
    self->solid = SOLID_NOT;
    self->svflags |= SVF_NOCLIENT;
  }

  SV_LinkEdict(self);
}

/* ===================================================== */

/*
 * QUAKED func_object (0 .5 .8) ? TRIGGER_SPAWN ANIMATED ANIMATED_FAST
 * This is solid bmodel that will fall if it's support it removed.
 */
void func_object_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
  if (!self || !other || !plane) {
    return;
  }

  if (plane->normal[2] < 1.0) {
    return;
  }

  if (other->takedamage == DAMAGE_NO) {
    return;
  }

  T_Damage(other, self, self, vec3_origin, self->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);
}

void func_object_release(edict_t *self)
{
  if (!self) {
    return;
  }

  self->movetype = MOVETYPE_TOSS;
  self->touch = func_object_touch;
}

void func_object_use(edict_t *self, edict_t *other, edict_t *activator)
{
  if (!self) {
    return;
  }

  self->solid = SOLID_BSP;
  self->svflags &= ~SVF_NOCLIENT;
  self->use = NULL;
  KillBox(self);
  func_object_release(self);
}

void SP_func_object(edict_t *self)
{
  if (!self) {
    return;
  }

  PF_setmodel(self, self->model);

  self->mins[0] += 1;
  self->mins[1] += 1;
  self->mins[2] += 1;
  self->maxs[0] -= 1;
  self->maxs[1] -= 1;
  self->maxs[2] -= 1;

  if (!self->dmg) {
    self->dmg = 100;
  }

  if (self->spawnflags == 0) {
    self->solid = SOLID_BSP;
    self->movetype = MOVETYPE_PUSH;
    self->think = func_object_release;
    self->nextthink = level.time + 2 * FRAMETIME;
  } else {
    self->solid = SOLID_NOT;
    self->movetype = MOVETYPE_PUSH;
    self->use = func_object_use;
    self->svflags |= SVF_NOCLIENT;
  }

  if (self->spawnflags & 2) {
    self->s.effects |= EF_ANIM_ALL;
  }

  if (self->spawnflags & 4) {
    self->s.effects |= EF_ANIM_ALLFAST;
  }

  self->clipmask = MASK_MONSTERSOLID;

  SV_LinkEdict(self);
}

/* ===================================================== */

/*
 * QUAKED func_explosive (0 .5 .8) ? Trigger_Spawn ANIMATED ANIMATED_FAST
 * Any brush that you want to explode or break apart. If you want an
 * explosion, set dmg and it will do a radius explosion of that amount
 * at the center of the bursh.
 *
 * If targeted it will not be shootable.
 *
 * health defaults to 100.
 *
 * mass defaults to 75.  This determines how much debris is emitted when
 * it explodes.  You get one large chunk per 100 of mass (up to 8) and
 * one small chunk per 25 of mass (up to 16).  So 800 gives the most.
 */
void func_explosive_explode(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
  vec3_t origin;
  vec3_t chunkorigin;
  vec3_t size;
  int count;
  int mass;

  if (!self || !inflictor || !attacker) {
    return;
  }

  /* bmodel origins are (0 0 0), we need to adjust that here */
  VectorScale(self->size, 0.5, size);
  VectorAdd(self->absmin, size, origin);
  VectorCopy(origin, self->s.origin);

  self->takedamage = DAMAGE_NO;

  if (self->dmg) {
    T_RadiusDamage(self, attacker, self->dmg, NULL, self->dmg + 40, MOD_EXPLOSIVE);
  }

  VectorSubtract(self->s.origin, inflictor->s.origin, self->velocity);
  VectorNormalize(self->velocity);
  VectorScale(self->velocity, 150, self->velocity);

  /* start chunks towards the center */
  VectorScale(size, 0.5, size);

  mass = self->mass;

  if (!mass) {
    mass = 75;
  }

  /* big chunks */
  if (mass >= 100) {
    count = mass / 100;

    if (count > 8) {
      count = 8;
    }

    while (count--) {
      chunkorigin[0] = origin[0] + crandom() * size[0];
      chunkorigin[1] = origin[1] + crandom() * size[1];
      chunkorigin[2] = origin[2] + crandom() * size[2];
      ThrowDebris(self, "models/objects/debris1/tris.md2", 1, chunkorigin);
    }
  }

  /* small chunks */
  count = mass / 25;

  if (count > 16) {
    count = 16;
  }

  while (count--) {
    chunkorigin[0] = origin[0] + crandom() * size[0];
    chunkorigin[1] = origin[1] + crandom() * size[1];
    chunkorigin[2] = origin[2] + crandom() * size[2];
    ThrowDebris(self, "models/objects/debris2/tris.md2", 2, chunkorigin);
  }

  G_UseTargets(self, attacker);

  if (self->dmg) {
    BecomeExplosion1(self);
  } else {
    G_FreeEdict(self);
  }
}

void func_explosive_use(edict_t *self, edict_t *other, edict_t *activator)
{
  func_explosive_explode(self, self, other, self->health, vec3_origin);
}

void func_explosive_spawn(edict_t *self, edict_t *other, edict_t *activator)
{
  self->solid = SOLID_BSP;
  self->svflags &= ~SVF_NOCLIENT;
  self->use = NULL;
  KillBox(self);
  SV_LinkEdict(self);
}

void SP_func_explosive(edict_t *self)
{
  if (!self) {
    return;
  }

  if (deathmatch->value) {
    /* auto-remove for deathmatch */
    G_FreeEdict(self);
    return;
  }

  self->movetype = MOVETYPE_PUSH;

  SV_ModelIndex("models/objects/debris1/tris.md2");
  SV_ModelIndex("models/objects/debris2/tris.md2");

  PF_setmodel(self, self->model);

  if (self->spawnflags & 1) {
    self->svflags |= SVF_NOCLIENT;
    self->solid = SOLID_NOT;
    self->use = func_explosive_spawn;
  } else {
    self->solid = SOLID_BSP;

    if (self->targetname) {
      self->use = func_explosive_use;
    }
  }

  if (self->spawnflags & 2) {
    self->s.effects |= EF_ANIM_ALL;
  }

  if (self->spawnflags & 4) {
    self->s.effects |= EF_ANIM_ALLFAST;
  }

  if (self->use != func_explosive_use) {
    if (!self->health) {
      self->health = 100;
    }

    self->die = func_explosive_explode;
    self->takedamage = DAMAGE_YES;
  }

  SV_LinkEdict(self);
}

/* ===================================================== */

/*
 * QUAKED misc_explobox (0 .5 .8) (-16 -16 0) (16 16 40)
 * Large exploding box.  You can override its mass (100),
 * health (80), and dmg (150).
 */

void barrel_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
  float ratio;
  vec3_t v;

  if (!self || !other) {
    return;
  }

  if ((!other->groundentity) || (other->groundentity == self)) {
    return;
  }

  ratio = (float) other->mass / (float) self->mass;
  VectorSubtract(self->s.origin, other->s.origin, v);
  M_walkmove(self, vectoyaw(v), 20 * ratio * FRAMETIME);
}

void barrel_explode(edict_t *self)
{
  vec3_t org;
  float spd;
  vec3_t save;

  if (!self) {
    return;
  }

  T_RadiusDamage(self, self->activator, self->dmg, NULL, self->dmg + 40, MOD_BARREL);
  VectorCopy(self->s.origin, save);
  VectorMA(self->absmin, 0.5, self->size, self->s.origin);

  /* a few big chunks */
  spd = 1.5 * (float) self->dmg / 200.0;
  org[0] = self->s.origin[0] + crandom() * self->size[0];
  org[1] = self->s.origin[1] + crandom() * self->size[1];
  org[2] = self->s.origin[2] + crandom() * self->size[2];
  ThrowDebris(self, "models/objects/debris1/tris.md2", spd, org);
  org[0] = self->s.origin[0] + crandom() * self->size[0];
  org[1] = self->s.origin[1] + crandom() * self->size[1];
  org[2] = self->s.origin[2] + crandom() * self->size[2];
  ThrowDebris(self, "models/objects/debris1/tris.md2", spd, org);

  /* bottom corners */
  spd = 1.75 * (float) self->dmg / 200.0;
  VectorCopy(self->absmin, org);
  ThrowDebris(self, "models/objects/debris3/tris.md2", spd, org);
  VectorCopy(self->absmin, org);
  org[0] += self->size[0];
  ThrowDebris(self, "models/objects/debris3/tris.md2", spd, org);
  VectorCopy(self->absmin, org);
  org[1] += self->size[1];
  ThrowDebris(self, "models/objects/debris3/tris.md2", spd, org);
  VectorCopy(self->absmin, org);
  org[0] += self->size[0];
  org[1] += self->size[1];
  ThrowDebris(self, "models/objects/debris3/tris.md2", spd, org);

  /* a bunch of little chunks */
  spd = 2 * self->dmg / 200;
  org[0] = self->s.origin[0] + crandom() * self->size[0];
  org[1] = self->s.origin[1] + crandom() * self->size[1];
  org[2] = self->s.origin[2] + crandom() * self->size[2];
  ThrowDebris(self, "models/objects/debris2/tris.md2", spd, org);
  org[0] = self->s.origin[0] + crandom() * self->size[0];
  org[1] = self->s.origin[1] + crandom() * self->size[1];
  org[2] = self->s.origin[2] + crandom() * self->size[2];
  ThrowDebris(self, "models/objects/debris2/tris.md2", spd, org);
  org[0] = self->s.origin[0] + crandom() * self->size[0];
  org[1] = self->s.origin[1] + crandom() * self->size[1];
  org[2] = self->s.origin[2] + crandom() * self->size[2];
  ThrowDebris(self, "models/objects/debris2/tris.md2", spd, org);
  org[0] = self->s.origin[0] + crandom() * self->size[0];
  org[1] = self->s.origin[1] + crandom() * self->size[1];
  org[2] = self->s.origin[2] + crandom() * self->size[2];
  ThrowDebris(self, "models/objects/debris2/tris.md2", spd, org);
  org[0] = self->s.origin[0] + crandom() * self->size[0];
  org[1] = self->s.origin[1] + crandom() * self->size[1];
  org[2] = self->s.origin[2] + crandom() * self->size[2];
  ThrowDebris(self, "models/objects/debris2/tris.md2", spd, org);
  org[0] = self->s.origin[0] + crandom() * self->size[0];
  org[1] = self->s.origin[1] + crandom() * self->size[1];
  org[2] = self->s.origin[2] + crandom() * self->size[2];
  ThrowDebris(self, "models/objects/debris2/tris.md2", spd, org);
  org[0] = self->s.origin[0] + crandom() * self->size[0];
  org[1] = self->s.origin[1] + crandom() * self->size[1];
  org[2] = self->s.origin[2] + crandom() * self->size[2];
  ThrowDebris(self, "models/objects/debris2/tris.md2", spd, org);
  org[0] = self->s.origin[0] + crandom() * self->size[0];
  org[1] = self->s.origin[1] + crandom() * self->size[1];
  org[2] = self->s.origin[2] + crandom() * self->size[2];
  ThrowDebris(self, "models/objects/debris2/tris.md2", spd, org);

  VectorCopy(save, self->s.origin);

  if (self->groundentity) {
    BecomeExplosion2(self);
  } else {
    BecomeExplosion1(self);
  }
}

void barrel_delay(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
  if (!self || !attacker) {
    return;
  }

  self->takedamage = DAMAGE_NO;
  self->nextthink = level.time + 2 * FRAMETIME;
  self->think = barrel_explode;
  self->activator = attacker;
}

void SP_misc_explobox(edict_t *self)
{
  if (!self) {
    return;
  }

  if (deathmatch->value) {
    /* auto-remove for deathmatch */
    G_FreeEdict(self);
    return;
  }

  SV_ModelIndex("models/objects/debris1/tris.md2");
  SV_ModelIndex("models/objects/debris2/tris.md2");
  SV_ModelIndex("models/objects/debris3/tris.md2");

  self->solid = SOLID_BBOX;
  self->movetype = MOVETYPE_STEP;

  self->model = "models/objects/barrels/tris.md2";
  self->s.modelindex = SV_ModelIndex(self->model);
  VectorSet(self->mins, -16, -16, 0);
  VectorSet(self->maxs, 16, 16, 40);

  if (!self->mass) {
    self->mass = 400;
  }

  if (!self->health) {
    self->health = 10;
  }

  if (!self->dmg) {
    self->dmg = 150;
  }

  self->die = barrel_delay;
  self->takedamage = DAMAGE_YES;
  self->monsterinfo.aiflags = AI_NOSTEP;

  self->touch = barrel_touch;

  self->think = M_droptofloor;
  self->nextthink = level.time + 2 * FRAMETIME;

  SV_LinkEdict(self);
}

/* ===================================================== */

/*
 * QUAKED target_character (0 0 1) ?
 * used with target_string (must be on same "team")
 * "count" is position in the string (starts at 1)
 */
void SP_target_character(edict_t *self)
{
  if (!self) {
    return;
  }

  self->movetype = MOVETYPE_PUSH;
  PF_setmodel(self, self->model);
  self->solid = SOLID_BSP;
  self->s.frame = 12;
  SV_LinkEdict(self);
  return;
}

/* ===================================================== */

/*
 * QUAKED target_string (0 0 1) (-8 -8 -8) (8 8 8)
 */
void target_string_use(edict_t *self, edict_t *other, edict_t *activator)
{
  edict_t *e;
  int n, l;
  char c;

  if (!self) {
    return;
  }

  l = strlen(self->message);

  for (e = self->teammaster; e; e = e->teamchain) {
    if (!e->count) {
      continue;
    }

    n = e->count - 1;

    if (n > l) {
      e->s.frame = 12;
      continue;
    }

    c = self->message[n];

    if ((c >= '0') && (c <= '9')) {
      e->s.frame = c - '0';
    } else if (c == '-') {
      e->s.frame = 10;
    } else if (c == ':') {
      e->s.frame = 11;
    } else {
      e->s.frame = 12;
    }
  }
}

void SP_target_string(edict_t *self)
{
  if (!self->message) {
    self->message = "";
  }

  self->use = target_string_use;
}

/* =================================================================================
 */

void teleporter_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
  edict_t *dest;
  int i;

  if (!self || !other) {
    return;
  }

  if (!other->client) {
    return;
  }

  dest = G_Find(NULL, FOFS(targetname), self->target);

  if (!dest) {
    PF_dprintf("Couldn't find destination\n");
    return;
  }

  /* unlink to make sure it can't possibly interfere with KillBox */
  SV_UnlinkEdict(other);

  VectorCopy(dest->s.origin, other->s.origin);
  VectorCopy(dest->s.origin, other->s.old_origin);
  other->s.origin[2] += 10;

  /* clear the velocity and hold them in place briefly */
  VectorClear(other->velocity);
  other->client->ps.pmove.pm_time = 160 >> 3; /* hold time */
  other->client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT;

  /* draw the teleport splash at source and on the player */
  self->owner->s.event = EV_PLAYER_TELEPORT;
  other->s.event = EV_PLAYER_TELEPORT;

  /* set angles */
  for (i = 0; i < 3; i++) {
    other->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(dest->s.angles[i] - other->client->resp.cmd_angles[i]);
  }

  VectorClear(other->s.angles);
  VectorClear(other->client->ps.viewangles);
  VectorClear(other->client->v_angle);

  /* kill anything at the destination */
  KillBox(other);

  SV_LinkEdict(other);
}

/*
 * QUAKED misc_teleporter (1 0 0) (-32 -32 -24) (32 32 -16)
 * Stepping onto this disc will teleport players to the targeted
 * misc_teleporter_dest object.
 */
void SP_misc_teleporter(edict_t *ent)
{
  if (!ent) {
    return;
  }

  edict_t *trig;

  if (!ent->target) {
    PF_dprintf("teleporter without a target.\n");
    G_FreeEdict(ent);
    return;
  }

  PF_setmodel(ent, "models/objects/dmspot/tris.md2");
  ent->s.skinnum = 1;
  ent->s.effects = EF_TELEPORTER;
  ent->s.sound = SV_SoundIndex("world/amb10.wav");
  ent->solid = SOLID_BBOX;

  VectorSet(ent->mins, -32, -32, -24);
  VectorSet(ent->maxs, 32, 32, -16);
  SV_LinkEdict(ent);

  trig = G_Spawn();
  trig->touch = teleporter_touch;
  trig->solid = SOLID_TRIGGER;
  trig->target = ent->target;
  trig->owner = ent;
  VectorCopy(ent->s.origin, trig->s.origin);
  VectorSet(trig->mins, -8, -8, 8);
  VectorSet(trig->maxs, 8, 8, 24);
  SV_LinkEdict(trig);
}

/*
 * QUAKED misc_teleporter_dest (1 0 0) (-32 -32 -24) (32 32 -16)
 * Point teleporters at these.
 */
void SP_misc_teleporter_dest(edict_t *ent)
{
  if (!ent) {
    return;
  }

  PF_setmodel(ent, "models/objects/dmspot/tris.md2");
  ent->s.skinnum = 0;
  ent->solid = SOLID_BBOX;
  VectorSet(ent->mins, -32, -32, -24);
  VectorSet(ent->maxs, 32, 32, -16);
  SV_LinkEdict(ent);
}
