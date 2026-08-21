#ifndef PLATFORM_THREAD_H
#define PLATFORM_THREAD_H

#include <stdint.h>
#include "../common/header/common.h"

void thread_pool_init(void);
void thread_pool_shutdown(void);
int thread_pool_workers(void);
void thread_pool_run(void (*work)(int unit), int units);

#endif // PLATFORM_THREAD_H
