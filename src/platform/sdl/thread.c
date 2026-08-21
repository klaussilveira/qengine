#include <SDL.h>
#include "../thread.h"

#define MAX_WORKERS 3

static SDL_Thread *workers[MAX_WORKERS];
static SDL_mutex *pool_mutex;
static SDL_cond *pool_start;
static SDL_cond *pool_done;
static SDL_atomic_t pool_next_unit;
static void (*pool_work)(int unit);
static int pool_units;
static int pool_worker_count;
static int pool_serial;
static int pool_finished;
static qboolean pool_shutdown;

static void thread_pool_drain(void)
{
  int unit;

  while ((unit = SDL_AtomicAdd(&pool_next_unit, 1)) < pool_units) {
    pool_work(unit);
  }
}

static int thread_pool_worker(void *data)
{
  int serial = 0;

  (void) data;

  for (;;) {
    SDL_LockMutex(pool_mutex);

    while (!pool_shutdown && serial == pool_serial) {
      SDL_CondWait(pool_start, pool_mutex);
    }

    if (pool_shutdown) {
      SDL_UnlockMutex(pool_mutex);

      return 0;
    }

    serial = pool_serial;
    SDL_UnlockMutex(pool_mutex);

    thread_pool_drain();

    SDL_LockMutex(pool_mutex);
    pool_finished++;

    if (pool_finished == pool_worker_count) {
      SDL_CondSignal(pool_done);
    }

    SDL_UnlockMutex(pool_mutex);
  }
}

void thread_pool_init(void)
{
  int desired;
  int i;

  if (pool_mutex) {
    return;
  }

  pool_shutdown = false;
  pool_mutex = SDL_CreateMutex();
  pool_start = SDL_CreateCond();
  pool_done = SDL_CreateCond();

  if (!pool_mutex || !pool_start || !pool_done) {
    thread_pool_shutdown();

    return;
  }

  desired = SDL_GetCPUCount() - 1;
  if (desired > MAX_WORKERS) {
    desired = MAX_WORKERS;
  }

  for (i = 0; i < desired; i++) {
    workers[i] = SDL_CreateThread(thread_pool_worker, "surface-cache", NULL);

    if (!workers[i]) {
      break;
    }

    pool_worker_count++;
  }
}

void thread_pool_shutdown(void)
{
  int i;

  if (pool_mutex) {
    SDL_LockMutex(pool_mutex);
    pool_shutdown = true;
    SDL_CondBroadcast(pool_start);
    SDL_UnlockMutex(pool_mutex);
  }

  for (i = 0; i < pool_worker_count; i++) {
    SDL_WaitThread(workers[i], NULL);
    workers[i] = NULL;
  }

  if (pool_done) {
    SDL_DestroyCond(pool_done);
  }
  pool_done = NULL;

  if (pool_start) {
    SDL_DestroyCond(pool_start);
  }
  pool_start = NULL;

  if (pool_mutex) {
    SDL_DestroyMutex(pool_mutex);
  }
  pool_mutex = NULL;

  pool_worker_count = 0;
  pool_serial = 0;
}

int thread_pool_workers(void)
{
  return pool_worker_count;
}

void thread_pool_run(void (*work)(int unit), int units)
{
  int unit;

  if (pool_worker_count == 0) {
    for (unit = 0; unit < units; unit++) {
      work(unit);
    }

    return;
  }

  pool_work = work;
  pool_units = units;
  pool_finished = 0;
  SDL_AtomicSet(&pool_next_unit, 0);

  SDL_LockMutex(pool_mutex);
  pool_serial++;
  SDL_CondBroadcast(pool_start);
  SDL_UnlockMutex(pool_mutex);

  thread_pool_drain();

  SDL_LockMutex(pool_mutex);
  while (pool_finished != pool_worker_count) {
    SDL_CondWait(pool_done, pool_mutex);
  }
  SDL_UnlockMutex(pool_mutex);
}
