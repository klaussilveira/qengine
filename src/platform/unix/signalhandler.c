/*
 * Copyright (C) 2011 Yamagi Burmeister
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
 */

#include <signal.h>

#ifdef __linux__

#include <execinfo.h>

#endif

#include "../../common/header/common.h"

#ifdef __linux__

void printBacktrace(int sig)
{
  void *array[15];
  int size;
  char **strings;
  int i;

  size = backtrace(array, 15);
  strings = backtrace_symbols(array, size);

  printf("Version:      %s\n", ENGINE_VERSION);
  printf("Compiler:     %s\n", __VERSION__);
  printf("Signal:       %i\n", sig);
  printf("\nBacktrace:\n");

  for (i = 0; i < size; i++) {
    printf("  %s\n", strings[i]);
  }

  printf("\n");
}

#else

void printBacktrace(int sig)
{
  printf("Version:      %s\n", ENGINE_VERSION);
  printf("Compiler:     %s\n", __VERSION__);
  printf("Signal:       %i\n", sig);
}

#endif

void signalhandler(int sig)
{
  printBacktrace(sig);
  fflush(stdout);

  signal(SIGSEGV, SIG_DFL);
  signal(SIGILL, SIG_DFL);
  signal(SIGFPE, SIG_DFL);
  signal(SIGABRT, SIG_DFL);

  raise(sig);
}

void terminate(int sig)
{
  Cbuf_AddText("quit");
}

void registerHandler(void)
{
  signal(SIGSEGV, signalhandler);
  signal(SIGILL, signalhandler);
  signal(SIGFPE, signalhandler);
  signal(SIGABRT, signalhandler);

  signal(SIGINT, terminate);
  signal(SIGTERM, terminate);
}
