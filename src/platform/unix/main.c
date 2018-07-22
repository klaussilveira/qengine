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
 * This file is the starting point of the program and implements
 * the main loop
 *
 * =======================================================================
 */

#include <fcntl.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "../../common/header/common.h"
#include "header/unix.h"

int main(int argc, char **argv)
{
  int verLen, i;
  long long oldtime, newtime;
  const char *versionString;

  // Time slept each frame.
#ifndef DEDICATED_ONLY
  struct timespec t = {0, 5000};
#else
  struct timespec t = {0, 850000};
#endif

  registerHandler();
  Sys_SetupFPU();

  if (getuid() != geteuid()) {
    printf("Effective UID does not match real UID.\n");

    return 1;
  }

  setenv("LC_ALL", "C", 1);

  versionString = va("qengine v%s", ENGINE_VERSION);
  verLen = strlen(versionString);

  printf("\n%s\n", versionString);
  for (i = 0; i < verLen; ++i) {
    putc('=', stdout);
  }
  puts("\n");

  /* Seed PRNG */
  randk_seed();

  /* Initialze the game */
  Qcommon_Init(argc, argv);

  /* Do not delay reads on stdin*/
  fcntl(fileno(stdin), F_SETFL, fcntl(fileno(stdin), F_GETFL, NULL) | FNDELAY);

  oldtime = Sys_Microseconds();

  /* The mainloop. The legend. */
  while (1) {
    // Throttle the game a little bit.
    nanosleep(&t, NULL);

    newtime = Sys_Microseconds();
    Qcommon_Frame(newtime - oldtime);
    oldtime = newtime;
  }

  return 0;
}
