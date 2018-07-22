#ifndef PLATFORM_INPUT_H
#define PLATFORM_INPUT_H

#include <stdint.h>

#include "../client/header/keyboard.h"
#include "../client/header/client.h"
#include "../common/header/common.h"
#include "graphics.h"

void input_init();
void input_command(usercmd_t *cmd);
void input_shutdown();
void input_update();

#endif // PLATFORM_INPUT_H
