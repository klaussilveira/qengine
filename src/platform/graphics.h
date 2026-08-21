#ifndef PLATFORM_GRAPHICS_H
#define PLATFORM_GRAPHICS_H

#include <stdint.h>
#include "../common/header/common.h"
#include "../client/renderer/header/local.h"

qboolean gfx_init();
void gfx_get_desktop_size(int *width, int *height);
qboolean gfx_create_window(int fullscreen, qboolean vsync, int win_width, int win_height);
qboolean gfx_resize_render_target(int render_width, int render_height);
void gfx_resize_window(int width, int height);
void gfx_get_drawable_size(int *width, int *height);
void gfx_window_grab_input(qboolean grab);
qboolean gfx_window_has_focus(void);
qboolean gfx_set_fullscreen(int fullscreen);
qboolean gfx_set_vsync(qboolean vsync);
qboolean gfx_set_refresh_rate(int refresh_rate, int width, int height);
void gfx_update(swstate_t sw_state, viddef_t vid);
int gfx_is_fullscreen();
int gfx_get_refresh_rate();
void gfx_reset_refresh_rate();
float gfx_get_ticks();
const char *gfx_get_error();
void gfx_free();
void gfx_shutdown();

#endif // PLATFORM_GRAPHICS_H
