/*
 * SDL backend for OpenGL renderer
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include "../graphics.h"
#include "../../client/header/vid.h"

static SDL_Window *window = NULL;
static int refreshRate = -1;
static qboolean initSuccessful = false;

/* Forward declarations for GL renderer functions (from gl1_sdl.c) */
extern int RI_PrepareForWindow(void);
extern int RI_InitContext(void *win);
extern void RE_ShutdownContext(void);

qboolean gfx_init()
{
	if (!SDL_WasInit(SDL_INIT_VIDEO)) {
		if (SDL_Init(SDL_INIT_VIDEO) == -1) {
			Com_Printf("Couldn't init SDL video: %s.\n", SDL_GetError());
			return false;
		}

		SDL_version version;
		SDL_GetVersion(&version);
		const char *driverName = SDL_GetCurrentVideoDriver();
		Com_Printf("SDL version is: %i.%i.%i\n", (int)version.major, (int)version.minor, (int)version.patch);
		Com_Printf("SDL video driver is \"%s\".\n", driverName);
	}

	return true;
}

qboolean gfx_create_window(qboolean fullscreen, qboolean vsync, int width, int height)
{
	/* For GL renderer, window creation is handled by GLimp_InitGraphics */
	return true;
}

void gfx_window_grab_input(qboolean grab)
{
	if (window != NULL) {
		SDL_SetWindowGrab(window, grab ? SDL_TRUE : SDL_FALSE);
	}

	if (SDL_SetRelativeMouseMode(grab ? SDL_TRUE : SDL_FALSE) < 0) {
		Com_Printf("WARNING: Setting Relative Mousemode failed, reason: %s\n", SDL_GetError());
		Com_Printf("         You should probably update to SDL 2.0.3 or newer!\n");
	}
}

qboolean gfx_is_fullscreen()
{
	if (window == NULL) {
		return 0;
	}

	if (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN_DESKTOP) {
		return 1;
	} else if (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN) {
		return 2;
	} else {
		return 0;
	}
}

qboolean gfx_update_fullscreen(qboolean fullscreen)
{
	Uint32 flags = 0;

	if (fullscreen == 1) {
		flags = SDL_WINDOW_FULLSCREEN_DESKTOP;
	} else if (fullscreen == 2) {
		flags = SDL_WINDOW_FULLSCREEN;
	}

	if (fullscreen != gfx_is_fullscreen()) {
		SDL_SetWindowFullscreen(window, flags);
		Cvar_SetValue("vid_fullscreen", fullscreen);
		return true;
	}

	return false;
}

void gfx_update(void)
{
	/* GL renderer handles frame presentation via RE_EndFrame() */
}

float gfx_get_ticks()
{
	return SDL_GetTicks();
}

const char *gfx_get_error()
{
	return SDL_GetError();
}

void gfx_free()
{
	if (window) {
		RE_ShutdownContext();
		gfx_window_grab_input(false);
		SDL_DestroyWindow(window);
		window = NULL;
	}

	refreshRate = -1;
	initSuccessful = false;
}

void gfx_shutdown()
{
	if (SDL_WasInit(SDL_INIT_EVERYTHING) == SDL_INIT_VIDEO) {
		SDL_Quit();
	} else {
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
	}
}

int gfx_get_refresh_rate()
{
	if (refreshRate == -1) {
		SDL_DisplayMode mode;

		int i = SDL_GetWindowDisplayIndex(window);
		if (i >= 0 && SDL_GetCurrentDisplayMode(i, &mode) == 0) {
			refreshRate = mode.refresh_rate;
		}

		if (refreshRate <= 0) {
			refreshRate = 60;
		}
	}

	return refreshRate;
}

void gfx_reset_refresh_rate()
{
	refreshRate = -1;
}

/*
 * Called by GL renderer to initialize graphics
 */
qboolean GLimp_InitGraphics(int fullscreen, int *pwidth, int *pheight)
{
	int flags;
	int width = *pwidth;
	int height = *pheight;
	unsigned int fs_flag = 0;

	if (fullscreen == 1) {
		fs_flag = SDL_WINDOW_FULLSCREEN_DESKTOP;
	} else if (fullscreen == 2) {
		fs_flag = SDL_WINDOW_FULLSCREEN;
	}

	/* Shut down existing window if present */
	if (window) {
		RE_ShutdownContext();
		gfx_window_grab_input(false);
		SDL_DestroyWindow(window);
		window = NULL;
	}

	/* Vid dimensions are needed for menu, HUD, viewport setup */
	vid.width = width;
	vid.height = height;

	/* Reset GL attributes */
	SDL_GL_ResetAttributes();

	/* Let renderer prepare GL attributes and get window flags */
	flags = RI_PrepareForWindow();
	if (flags == -1) {
		return false;
	}

	if (fs_flag) {
		flags |= fs_flag;
	}

	/* Create the window */
	int windowPos = SDL_WINDOWPOS_CENTERED;
	window = SDL_CreateWindow("qengine", windowPos, windowPos, width, height, flags);

	if (window == NULL) {
		Com_Printf("SDL CreateWindow failed: %s\n", SDL_GetError());

		/* Try fallback to 640x480 windowed */
		if (width != 640 || height != 480 || fs_flag) {
			Com_Printf("Reverting to windowed 640x480.\n");

			Cvar_SetValue("r_mode", 4);
			Cvar_SetValue("vid_fullscreen", 0);

			*pwidth = width = 640;
			*pheight = height = 480;
			flags &= ~fs_flag;

			window = SDL_CreateWindow("qengine", windowPos, windowPos, width, height, flags);
		}

		if (window == NULL) {
			Com_Error(ERR_FATAL, "Failed to create window. Exiting...\n");
			return false;
		}
	}

	if (!RI_InitContext(window)) {
		return false;
	}

	VID_NewWindow(width, height);
	SDL_ShowCursor(0);

	initSuccessful = true;
	refreshRate = -1;

	return true;
}
