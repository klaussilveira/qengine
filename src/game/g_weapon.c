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
 * Weapon support functions.
 *
 * =======================================================================
 */

#include "../server/header/server.h"

/*
 * This is a support routine used when a client is firing
 * a non-instant attack weapon.  It checks to see if a
 * monster's dodge function should be called.
 */
void check_dodge(edict_t *self, vec3_t start, vec3_t dir, int speed)
{
  vec3_t end;
  vec3_t v;
  trace_t tr;
  float eta;

  if (!self) {
    return;
  }

  /* easy mode only ducks one quarter the time */
  if (skill->value == 0) {
    if (random() > 0.25) {
      return;
    }
  }

  VectorMA(start, 8192, dir, end);
  tr = SV_Trace(start, NULL, NULL, end, self, MASK_SHOT);

  if ((tr.ent) && (tr.ent->svflags & SVF_MONSTER) && (tr.ent->health > 0) && (tr.ent->monsterinfo.dodge) &&
      infront(tr.ent, self)) {
    VectorSubtract(tr.endpos, start, v);
    eta = (VectorLength(v) - tr.ent->maxs[0]) / speed;
    tr.ent->monsterinfo.dodge(tr.ent, self, eta);
  }
}

/*
 * Used for all impact (hit/punch/slash) attacks
 */
qboolean fire_hit(edict_t *self, vec3_t aim, int damage, int kick)
{
  trace_t tr;
  vec3_t forward, right, up;
  vec3_t v;
  vec3_t point;
  float range;
  vec3_t dir;

  if (!self) {
    return false;
  }

  /* Lazarus: Paranoia check */
  if (!self->enemy) {
    return false;
  }

  /* see if enemy is in range */
  VectorSubtract(self->enemy->s.origin, self->s.origin, dir);
  range = VectorLength(dir);

  if (range > aim[0]) {
    return false;
  }

  if ((aim[1] > self->mins[0]) && (aim[1] < self->maxs[0])) {
    /* the hit is straight on so back the
       range up to the edge of their bbox */
    range -= self->enemy->maxs[0];
  } else {
    /* this is a side hit so adjust the "right"
       value out to the edge of their bbox */
    if (aim[1] < 0) {
      aim[1] = self->enemy->mins[0];
    } else {
      aim[1] = self->enemy->maxs[0];
    }
  }

  VectorMA(self->s.origin, range, dir, point);

  tr = SV_Trace(self->s.origin, NULL, NULL, point, self, MASK_SHOT);

  if (tr.fraction < 1) {
    if (!tr.ent->takedamage) {
      return false;
    }

    /* if it will hit any client/monster
       then hit the one we wanted to hit */
    if ((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client)) {
      tr.ent = self->enemy;
    }
  }

  AngleVectors(self->s.angles, forward, right, up);
  VectorMA(self->s.origin, range, forward, point);
  VectorMA(point, aim[1], right, point);
  VectorMA(point, aim[2], up, point);
  VectorSubtract(point, self->enemy->s.origin, dir);

  /* do the damage */
  T_Damage(tr.ent, self, self, dir, point, vec3_origin, damage, kick / 2, DAMAGE_NO_KNOCKBACK, MOD_HIT);

  if (!(tr.ent->svflags & SVF_MONSTER) && (!tr.ent->client)) {
    return false;
  }

  /* do our special form of knockback here */
  VectorMA(self->enemy->absmin, 0.5, self->enemy->size, v);
  VectorSubtract(v, point, v);
  VectorNormalize(v);
  VectorMA(self->enemy->velocity, kick, v, self->enemy->velocity);

  if (self->enemy->velocity[2] > 0) {
    self->enemy->groundentity = NULL;
  }

  return true;
}

/*
 * This is an internal support routine
 * used for bullet/pellet based weapons.
 */
void fire_lead(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int te_impact, int hspread,
               int vspread, int mod)
{
  trace_t tr;
  vec3_t dir;
  vec3_t forward, right, up;
  vec3_t end;
  float r;
  float u;
  vec3_t water_start;
  qboolean water = false;
  int content_mask = MASK_SHOT | MASK_WATER;

  if (!self) {
    return;
  }

  tr = SV_Trace(self->s.origin, NULL, NULL, start, self, MASK_SHOT);

  if (tr.fraction >= 1.0) {
    vectoangles(aimdir, dir);
    AngleVectors(dir, forward, right, up);

    r = crandom() * hspread;
    u = crandom() * vspread;
    VectorMA(start, 8192, forward, end);
    VectorMA(end, r, right, end);
    VectorMA(end, u, up, end);

    if (SV_PointContents(start) & MASK_WATER) {
      water = true;
      VectorCopy(start, water_start);
      content_mask &= ~MASK_WATER;
    }

    tr = SV_Trace(start, NULL, NULL, end, self, content_mask);

    /* see if we hit water */
    if (tr.contents & MASK_WATER) {
      int color;

      water = true;
      VectorCopy(tr.endpos, water_start);

      if (!VectorCompare(start, tr.endpos)) {
        if (tr.contents & CONTENTS_WATER) {
          if (strcmp(tr.surface->name, "*brwater") == 0) {
            color = SPLASH_BROWN_WATER;
          } else {
            color = SPLASH_BLUE_WATER;
          }
        } else if (tr.contents & CONTENTS_SLIME) {
          color = SPLASH_SLIME;
        } else if (tr.contents & CONTENTS_LAVA) {
          color = SPLASH_LAVA;
        } else {
          color = SPLASH_UNKNOWN;
        }

        if (color != SPLASH_UNKNOWN) {
          PF_WriteByte(svc_temp_entity);
          PF_WriteByte(TE_SPLASH);
          PF_WriteByte(8);
          PF_WritePos(tr.endpos);
          PF_WriteDir(tr.plane.normal);
          PF_WriteByte(color);
          SV_Multicast(tr.endpos, MULTICAST_PVS);
        }

        /* change bullet's course when it enters water */
        VectorSubtract(end, start, dir);
        vectoangles(dir, dir);
        AngleVectors(dir, forward, right, up);
        r = crandom() * hspread * 2;
        u = crandom() * vspread * 2;
        VectorMA(water_start, 8192, forward, end);
        VectorMA(end, r, right, end);
        VectorMA(end, u, up, end);
      }

      /* re-trace ignoring water this time */
      tr = SV_Trace(water_start, NULL, NULL, end, self, MASK_SHOT);
    }
  }

  /* send gun puff / flash */
  if (!((tr.surface) && (tr.surface->flags & SURF_SKY))) {
    if (tr.fraction < 1.0) {
      if (tr.ent->takedamage) {
        T_Damage(tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, DAMAGE_BULLET, mod);
      } else {
        if (strncmp(tr.surface->name, "sky", 3) != 0) {
          PF_WriteByte(svc_temp_entity);
          PF_WriteByte(te_impact);
          PF_WritePos(tr.endpos);
          PF_WriteDir(tr.plane.normal);
          SV_Multicast(tr.endpos, MULTICAST_PVS);

          if (self->client) {
            PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
          }
        }
      }
    }
  }

  /* if went through water, determine
     where the end and make a bubble trail */
  if (water) {
    vec3_t pos;

    VectorSubtract(tr.endpos, water_start, dir);
    VectorNormalize(dir);
    VectorMA(tr.endpos, -2, dir, pos);

    if (SV_PointContents(pos) & MASK_WATER) {
      VectorCopy(pos, tr.endpos);
    } else {
      tr = SV_Trace(pos, NULL, NULL, water_start, tr.ent, MASK_WATER);
    }

    VectorAdd(water_start, tr.endpos, pos);
    VectorScale(pos, 0.5, pos);

    PF_WriteByte(svc_temp_entity);
    PF_WriteByte(TE_BUBBLETRAIL);
    PF_WritePos(water_start);
    PF_WritePos(tr.endpos);
    SV_Multicast(pos, MULTICAST_PVS);
  }
}

/*
 * Fires a single round.  Used for machinegun and
 * chaingun.  Would be fine for pistols, rifles, etc....
 */
void fire_bullet(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod)
{
  if (!self) {
    return;
  }

  fire_lead(self, start, aimdir, damage, kick, TE_GUNSHOT, hspread, vspread, mod);
}

/*
 * Shoots shotgun pellets. Used
 * by shotgun and super shotgun.
 */
void fire_shotgun(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int count,
                  int mod)
{
  int i;

  if (!self) {
    return;
  }

  for (i = 0; i < count; i++) {
    fire_lead(self, start, aimdir, damage, kick, TE_SHOTGUN, hspread, vspread, mod);
  }
}

/*
 * Fires a single blaster bolt.
 * Used by the blaster and hyper blaster.
 */
void blaster_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
  int mod;

  if (!self || !other) /* plane and surf can be NULL */
  {
    G_FreeEdict(self);
    return;
  }

  if (other == self->owner) {
    return;
  }

  if (surf && (surf->flags & SURF_SKY)) {
    G_FreeEdict(self);
    return;
  }

  if (self->owner && self->owner->client) {
    PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);
  }

  if (other->takedamage) {
    if (self->spawnflags & 1) {
      mod = MOD_HYPERBLASTER;
    } else {
      mod = MOD_BLASTER;
    }

    if (plane) {
      T_Damage(other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY,
               mod);
    } else {
      T_Damage(other, self, self->owner, self->velocity, self->s.origin, vec3_origin, self->dmg, 1, DAMAGE_ENERGY, mod);
    }
  } else {
    PF_WriteByte(svc_temp_entity);
    PF_WriteByte(TE_BLASTER);
    PF_WritePos(self->s.origin);

    if (!plane) {
      PF_WriteDir(vec3_origin);
    } else {
      PF_WriteDir(plane->normal);
    }

    SV_Multicast(self->s.origin, MULTICAST_PVS);
  }

  G_FreeEdict(self);
}

void fire_blaster(edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect, qboolean hyper)
{
  edict_t *bolt;
  trace_t tr;

  if (!self) {
    return;
  }

  VectorNormalize(dir);

  bolt = G_Spawn();
  bolt->svflags = SVF_DEADMONSTER;

  /* yes, I know it looks weird that projectiles are deadmonsters
     what this means is that when prediction is used against the object
     (blaster/hyperblaster shots), the player won't be solid clipped against
     the object.  Right now trying to run into a firing hyperblaster
     is very jerky since you are predicted 'against' the shots. */
  VectorCopy(start, bolt->s.origin);
  VectorCopy(start, bolt->s.old_origin);
  vectoangles(dir, bolt->s.angles);
  VectorScale(dir, speed, bolt->velocity);
  bolt->movetype = MOVETYPE_FLYMISSILE;
  bolt->clipmask = MASK_SHOT;
  bolt->solid = SOLID_BBOX;
  bolt->s.effects |= effect;
  bolt->s.renderfx |= RF_NOSHADOW;
  VectorClear(bolt->mins);
  VectorClear(bolt->maxs);
  bolt->s.modelindex = SV_ModelIndex("models/objects/laser/tris.md2");
  bolt->s.sound = SV_SoundIndex("misc/lasfly.wav");
  bolt->owner = self;
  bolt->touch = blaster_touch;
  bolt->nextthink = level.time + 2;
  bolt->think = G_FreeEdict;
  bolt->dmg = damage;
  bolt->classname = "bolt";

  if (hyper) {
    bolt->spawnflags = 1;
  }

  SV_LinkEdict(bolt);

  if (self->client) {
    check_dodge(self, bolt->s.origin, dir, speed);
  }

  tr = SV_Trace(self->s.origin, NULL, NULL, bolt->s.origin, bolt, MASK_SHOT);

  if (tr.fraction < 1.0) {
    VectorMA(bolt->s.origin, -10, dir, bolt->s.origin);
    bolt->touch(bolt, tr.ent, NULL, NULL);
  }
}

void Grenade_Explode(edict_t *ent)
{
  vec3_t origin;
  int mod;

  if (!ent) {
    return;
  }

  if (ent->owner && ent->owner->client) {
    PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);
  }

  if (ent->enemy) {
    float points;
    vec3_t v;
    vec3_t dir;

    VectorAdd(ent->enemy->mins, ent->enemy->maxs, v);
    VectorMA(ent->enemy->s.origin, 0.5, v, v);
    VectorSubtract(ent->s.origin, v, v);
    points = ent->dmg - 0.5 * VectorLength(v);
    VectorSubtract(ent->enemy->s.origin, ent->s.origin, dir);

    if (ent->spawnflags & 1) {
      mod = MOD_HANDGRENADE;
    } else {
      mod = MOD_GRENADE;
    }

    T_Damage(ent->enemy, ent, ent->owner, dir, ent->s.origin, vec3_origin, (int) points, (int) points, DAMAGE_RADIUS,
             mod);
  }

  if (ent->spawnflags & 2) {
    mod = MOD_HELD_GRENADE;
  } else if (ent->spawnflags & 1) {
    mod = MOD_HG_SPLASH;
  } else {
    mod = MOD_G_SPLASH;
  }

  T_RadiusDamage(ent, ent->owner, ent->dmg, ent->enemy, ent->dmg_radius, mod);

  VectorMA(ent->s.origin, -0.02, ent->velocity, origin);
  PF_WriteByte(svc_temp_entity);

  if (ent->waterlevel) {
    if (ent->groundentity) {
      PF_WriteByte(TE_GRENADE_EXPLOSION_WATER);
    } else {
      PF_WriteByte(TE_ROCKET_EXPLOSION_WATER);
    }
  } else {
    if (ent->groundentity) {
      PF_WriteByte(TE_GRENADE_EXPLOSION);
    } else {
      PF_WriteByte(TE_ROCKET_EXPLOSION);
    }
  }

  PF_WritePos(origin);
  SV_Multicast(ent->s.origin, MULTICAST_PHS);

  G_FreeEdict(ent);
}

void Grenade_Touch(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
  if (!ent || !other) /* plane is unused, surf can be NULL */
  {
    G_FreeEdict(ent);
    return;
  }

  if (other == ent->owner) {
    return;
  }

  if (surf && (surf->flags & SURF_SKY)) {
    G_FreeEdict(ent);
    return;
  }

  if (!other->takedamage) {
    if (ent->spawnflags & 1) {
      if (random() > 0.5) {
        PF_StartSound(ent, CHAN_VOICE, SV_SoundIndex("weapons/hgrenb1a.wav"), 1, ATTN_NORM, 0);
      } else {
        PF_StartSound(ent, CHAN_VOICE, SV_SoundIndex("weapons/hgrenb2a.wav"), 1, ATTN_NORM, 0);
      }
    } else {
      PF_StartSound(ent, CHAN_VOICE, SV_SoundIndex("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);
    }

    return;
  }

  ent->enemy = other;
  Grenade_Explode(ent);
}

void fire_grenade(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
{
  edict_t *grenade;
  vec3_t dir;
  vec3_t forward, right, up;

  if (!self) {
    return;
  }

  vectoangles(aimdir, dir);
  AngleVectors(dir, forward, right, up);

  grenade = G_Spawn();
  VectorCopy(start, grenade->s.origin);
  VectorScale(aimdir, speed, grenade->velocity);
  VectorMA(grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
  VectorMA(grenade->velocity, crandom() * 10.0, right, grenade->velocity);
  VectorSet(grenade->avelocity, 300, 300, 300);
  grenade->movetype = MOVETYPE_BOUNCE;
  grenade->clipmask = MASK_SHOT;
  grenade->solid = SOLID_BBOX;
  grenade->s.effects |= EF_GRENADE;
  VectorClear(grenade->mins);
  VectorClear(grenade->maxs);
  grenade->s.modelindex = SV_ModelIndex("models/objects/grenade/tris.md2");
  grenade->owner = self;
  grenade->touch = Grenade_Touch;
  grenade->nextthink = level.time + timer;
  grenade->think = Grenade_Explode;
  grenade->dmg = damage;
  grenade->dmg_radius = damage_radius;
  grenade->classname = "grenade";

  SV_LinkEdict(grenade);
}

void fire_grenade2(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius,
                   qboolean held)
{
  edict_t *grenade;
  vec3_t dir;
  vec3_t forward, right, up;

  if (!self) {
    return;
  }

  vectoangles(aimdir, dir);
  AngleVectors(dir, forward, right, up);

  grenade = G_Spawn();
  VectorCopy(start, grenade->s.origin);
  VectorScale(aimdir, speed, grenade->velocity);
  VectorMA(grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
  VectorMA(grenade->velocity, crandom() * 10.0, right, grenade->velocity);
  VectorSet(grenade->avelocity, 300, 300, 300);
  grenade->movetype = MOVETYPE_BOUNCE;
  grenade->clipmask = MASK_SHOT;
  grenade->solid = SOLID_BBOX;
  grenade->s.effects |= EF_GRENADE;
  VectorClear(grenade->mins);
  VectorClear(grenade->maxs);
  grenade->s.modelindex = SV_ModelIndex("models/objects/grenade2/tris.md2");
  grenade->owner = self;
  grenade->touch = Grenade_Touch;
  grenade->nextthink = level.time + timer;
  grenade->think = Grenade_Explode;
  grenade->dmg = damage;
  grenade->dmg_radius = damage_radius;
  grenade->classname = "hgrenade";

  if (held) {
    grenade->spawnflags = 3;
  } else {
    grenade->spawnflags = 1;
  }

  grenade->s.sound = SV_SoundIndex("weapons/hgrenc1b.wav");

  if (timer <= 0.0) {
    Grenade_Explode(grenade);
  } else {
    PF_StartSound(self, CHAN_WEAPON, SV_SoundIndex("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
    SV_LinkEdict(grenade);
  }
}

void rocket_touch(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
  vec3_t origin;
  int n;

  if (!ent || !other) /* plane and surf can be NULL */
  {
    G_FreeEdict(ent);
    return;
  }

  if (other == ent->owner) {
    return;
  }

  if (surf && (surf->flags & SURF_SKY)) {
    G_FreeEdict(ent);
    return;
  }

  if (ent->owner && ent->owner->client) {
    PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);
  }

  /* calculate position for the explosion entity */
  VectorMA(ent->s.origin, -0.02, ent->velocity, origin);

  if (other->takedamage) {
    if (plane) {
      T_Damage(other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_ROCKET);
    } else {
      T_Damage(other, ent, ent->owner, ent->velocity, ent->s.origin, vec3_origin, ent->dmg, 0, 0, MOD_ROCKET);
    }
  } else {
    /* don't throw any debris in net games */
    if (!deathmatch->value && !coop->value) {
      if ((surf) && !(surf->flags & (SURF_WARP | SURF_TRANS33 | SURF_TRANS66 | SURF_FLOWING))) {
        n = randk() % 5;

        while (n--) {
          ThrowDebris(ent, "models/objects/debris2/tris.md2", 2, ent->s.origin);
        }
      }
    }
  }

  T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, MOD_R_SPLASH);

  PF_WriteByte(svc_temp_entity);

  if (ent->waterlevel) {
    PF_WriteByte(TE_ROCKET_EXPLOSION_WATER);
  } else {
    PF_WriteByte(TE_ROCKET_EXPLOSION);
  }

  PF_WritePos(origin);
  SV_Multicast(ent->s.origin, MULTICAST_PHS);

  G_FreeEdict(ent);
}

void fire_rocket(edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
  edict_t *rocket;

  if (!self) {
    return;
  }

  rocket = G_Spawn();
  VectorCopy(start, rocket->s.origin);
  VectorCopy(dir, rocket->movedir);
  vectoangles(dir, rocket->s.angles);
  VectorScale(dir, speed, rocket->velocity);
  rocket->movetype = MOVETYPE_FLYMISSILE;
  rocket->clipmask = MASK_SHOT;
  rocket->solid = SOLID_BBOX;
  rocket->s.effects |= EF_ROCKET;
  VectorClear(rocket->mins);
  VectorClear(rocket->maxs);
  rocket->s.modelindex = SV_ModelIndex("models/objects/rocket/tris.md2");
  rocket->owner = self;
  rocket->touch = rocket_touch;
  rocket->nextthink = level.time + 8000 / speed;
  rocket->think = G_FreeEdict;
  rocket->dmg = damage;
  rocket->radius_dmg = radius_damage;
  rocket->dmg_radius = damage_radius;
  rocket->s.sound = SV_SoundIndex("weapons/rockfly.wav");
  rocket->classname = "rocket";

  if (self->client) {
    check_dodge(self, rocket->s.origin, dir, speed);
  }

  SV_LinkEdict(rocket);
}

void fire_rail(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick)
{
  vec3_t from;
  vec3_t end;
  trace_t tr;
  edict_t *ignore;
  int mask;
  qboolean water;

  if (!self) {
    return;
  }

  VectorMA(start, 8192, aimdir, end);
  VectorCopy(start, from);
  ignore = self;
  water = false;
  mask = MASK_SHOT | CONTENTS_SLIME | CONTENTS_LAVA;

  while (ignore) {
    tr = SV_Trace(from, NULL, NULL, end, ignore, mask);

    if (tr.contents & (CONTENTS_SLIME | CONTENTS_LAVA)) {
      mask &= ~(CONTENTS_SLIME | CONTENTS_LAVA);
      water = true;
    } else {
      if ((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client) || (tr.ent->solid == SOLID_BBOX)) {
        ignore = tr.ent;
      } else {
        ignore = NULL;
      }

      if ((tr.ent != self) && (tr.ent->takedamage)) {
        T_Damage(tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, 0, MOD_RAILGUN);
      } else {
        ignore = NULL;
      }
    }

    VectorCopy(tr.endpos, from);
  }

  /* send gun puff / flash */
  PF_WriteByte(svc_temp_entity);
  PF_WriteByte(TE_RAILTRAIL);
  PF_WritePos(start);
  PF_WritePos(tr.endpos);
  SV_Multicast(self->s.origin, MULTICAST_PHS);

  if (water) {
    PF_WriteByte(svc_temp_entity);
    PF_WriteByte(TE_RAILTRAIL);
    PF_WritePos(start);
    PF_WritePos(tr.endpos);
    SV_Multicast(tr.endpos, MULTICAST_PHS);
  }

  if (self->client) {
    PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
  }
}
