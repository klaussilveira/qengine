#ifndef PLATFORM_GRAPHICS_H
#define PLATFORM_GRAPHICS_H

#include <stdint.h>
#include "../common/header/common.h"
#include "../client/renderer/header/local.h"

qboolean gfx_init();
void gfx_get_desktop_size(int *width, int *height);
qboolean gfx_create_window(qboolean fullscreen, qboolean vsync, int win_width, int win_height, int render_width,
                           int render_height);
void gfx_window_grab_input(qboolean grab);
qboolean gfx_update_fullscreen(qboolean fullscreen);
void gfx_update(swstate_t sw_state, viddef_t vid);
qboolean gfx_is_fullscreen();
int gfx_get_refresh_rate();
void gfx_reset_refresh_rate();
float gfx_get_ticks();
const char *gfx_get_error();
void gfx_free();
void gfx_shutdown();

#endif // PLATFORM_GRAPHICS_H
