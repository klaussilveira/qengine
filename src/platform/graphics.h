#ifndef PLATFORM_GRAPHICS_H
#define PLATFORM_GRAPHICS_H

#include <stdint.h>
#include "../common/header/common.h"

#ifdef USE_GL_RENDERER
#include "../client/renderer/gl1/header/local.h"
#else
#include "../client/renderer/header/local.h"
#endif

qboolean gfx_init();
qboolean gfx_create_window(qboolean fullscreen, qboolean vsync, int width, int height);
void gfx_window_grab_input(qboolean grab);
qboolean gfx_update_fullscreen(qboolean fullscreen);
#ifndef USE_GL_RENDERER
void gfx_update(swstate_t sw_state, viddef_t vid);
#else
void gfx_update(void);
#endif
qboolean gfx_is_fullscreen();
int gfx_get_refresh_rate();
void gfx_reset_refresh_rate();
float gfx_get_ticks();
const char *gfx_get_error();
void gfx_free();
void gfx_shutdown();

#endif // PLATFORM_GRAPHICS_H
