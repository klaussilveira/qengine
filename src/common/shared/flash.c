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
 * Muzzle flash posititions.
 *
 * =======================================================================
 */

#include "../header/shared.h"

vec3_t muzzle_flash_offset[] = {{0.0, 0.0, 0.0},     // MONSTER_NONE
                                {26.6, 7.1, 13.1},   // MONSTER_INFANTRY
                                {12.72, 9.24, 9.24}, // MONSTER_SOLDIER_BLASTER
                                {12.72, 9.24, 9.24}, // MONSTER_SOLDIER_MACHINEGUN
                                {12.72, 9.24, 9.24}, // MONSTER_SOLDIER_SHOTGUN
                                {0.0, 0.0, 0.0}};
