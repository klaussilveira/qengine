#include <SDL.h>
#include "../graphics.h"

static SDL_Window *window = NULL;
static SDL_Surface *surface = NULL;
static SDL_Surface *surface_indexed = NULL; // 8-bit indexed source surface
static SDL_Texture *texture = NULL;
static SDL_Renderer *renderer = NULL;
static unsigned char cached_palette[1024];
static int refreshRate = -1;
static qboolean mouse_grabbed = false;

// Internal render resolution vs window resolution
static int render_width = 0;
static int render_height = 0;
static int window_width = 0;
static int window_height = 0;

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
    Com_Printf("SDL version is: %i.%i.%i\n", (int) version.major, (int) version.minor, (int) version.patch);
    Com_Printf("SDL video driver is \"%s\".\n", driverName);
  }

  return true;
}

void gfx_get_desktop_size(int *width, int *height)
{
  SDL_DisplayMode mode;

  if (SDL_GetDesktopDisplayMode(0, &mode) == 0) {
    *width = mode.w;
    *height = mode.h;
  } else {
    *width = 1920;
    *height = 1080;
    Com_Printf("Warning: Could not get desktop display mode: %s\n", SDL_GetError());
  }
}

static void DestroyRenderTarget(void)
{
  if (texture) {
    SDL_DestroyTexture(texture);
  }
  texture = NULL;

  if (surface) {
    SDL_FreeSurface(surface);
  }
  surface = NULL;

  if (surface_indexed) {
    SDL_FreeSurface(surface_indexed);
  }
  surface_indexed = NULL;

  memset(cached_palette, 0, sizeof(cached_palette));
}

qboolean gfx_resize_render_target(int rend_width, int rend_height)
{
  Uint32 Rmask, Gmask, Bmask, Amask;
  int bpp;

  if (renderer == NULL) {
    return false;
  }

  if (!SDL_PixelFormatEnumToMasks(SDL_PIXELFORMAT_ARGB8888, &bpp, &Rmask, &Gmask, &Bmask, &Amask)) {
    return false;
  }

  DestroyRenderTarget();

  render_width = rend_width;
  render_height = rend_height;

  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
  surface = SDL_CreateRGBSurface(0, render_width, render_height, bpp, Rmask, Gmask, Bmask, Amask);
  surface_indexed = SDL_CreateRGBSurface(0, render_width, render_height, 8, 0, 0, 0, 0);
  texture =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, render_width, render_height);

  if (surface == NULL || surface_indexed == NULL || texture == NULL) {
    Com_Printf("Failed to create render target %dx%d: %s\n", render_width, render_height, SDL_GetError());
    DestroyRenderTarget();

    return false;
  }

  Com_Printf("Render target: %dx%d\n", render_width, render_height);

  return true;
}

qboolean gfx_create_window(int fullscreen, qboolean vsync, int win_width, int win_height)
{
  Uint32 flags = SDL_SWSURFACE | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
  int windowPos = SDL_WINDOWPOS_CENTERED;

  window_width = win_width;
  window_height = win_height;

  if (fullscreen == 1) {
    flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
  } else if (fullscreen == 2) {
    flags |= SDL_WINDOW_FULLSCREEN;
  }

  window = SDL_CreateWindow("qengine", windowPos, windowPos, win_width, win_height, flags);
  if (window == NULL) {
    Com_Printf("Failed to create window: %s\n", SDL_GetError());
    return false;
  }

  SDL_SetWindowMinimumSize(window, MIN_RENDER_WIDTH, MIN_RENDER_HEIGHT);

  if (vsync) {
    renderer = SDL_CreateRenderer(window, -1,
                                  SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  } else {
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  }

  if (renderer == NULL) {
    Com_Printf("Failed to create renderer: %s\n", SDL_GetError());
    return false;
  }

  // Clear to black until it loads
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);
  SDL_RenderPresent(renderer);

  SDL_ShowCursor(0);

  Com_Printf("Window: %dx%d\n", win_width, win_height);

  return true;
}

void gfx_resize_window(int width, int height)
{
  if (window == NULL) {
    return;
  }

  SDL_SetWindowSize(window, width, height);
  SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
  window_width = width;
  window_height = height;
}

void gfx_get_drawable_size(int *width, int *height)
{
  if (renderer != NULL && SDL_GetRendererOutputSize(renderer, width, height) == 0) {
    return;
  }

  if (window != NULL) {
    SDL_GetWindowSize(window, width, height);
    return;
  }

  *width = window_width;
  *height = window_height;
}

qboolean gfx_window_has_focus(void)
{
  Uint32 flags;

  if (window == NULL) {
    return false;
  }

  flags = SDL_GetWindowFlags(window);

  return (flags & SDL_WINDOW_INPUT_FOCUS) != 0 && (flags & SDL_WINDOW_MINIMIZED) == 0;
}

void gfx_window_grab_input(qboolean grab)
{
  if (window == NULL || grab == mouse_grabbed) {
    return;
  }

  if (SDL_SetRelativeMouseMode(grab ? SDL_TRUE : SDL_FALSE) < 0) {
    Com_Printf("WARNING: Setting Relative Mousemode failed, reason: %s\n", SDL_GetError());
    Com_Printf("         You should probably update to SDL 2.0.3 or newer!\n");
    SDL_SetWindowGrab(window, SDL_FALSE);
    mouse_grabbed = false;

    return;
  }

  SDL_SetWindowGrab(window, grab ? SDL_TRUE : SDL_FALSE);
  SDL_FlushEvent(SDL_MOUSEMOTION);
  mouse_grabbed = grab;
}

int gfx_is_fullscreen()
{
  Uint32 flags;

  if (window == NULL) {
    return 0;
  }

  flags = SDL_GetWindowFlags(window);

  if ((flags & SDL_WINDOW_FULLSCREEN_DESKTOP) == SDL_WINDOW_FULLSCREEN_DESKTOP) {
    return 1;
  } else if (flags & SDL_WINDOW_FULLSCREEN) {
    return 2;
  } else {
    return 0;
  }
}

qboolean gfx_set_fullscreen(int fullscreen)
{
  Uint32 flags = 0;

  if (window == NULL) {
    return false;
  }

  if (fullscreen == 1) {
    flags = SDL_WINDOW_FULLSCREEN_DESKTOP;
  } else if (fullscreen == 2) {
    flags = SDL_WINDOW_FULLSCREEN;
  }

  if (SDL_SetWindowFullscreen(window, flags) < 0) {
    Com_Printf("Failed to change fullscreen mode: %s\n", SDL_GetError());
    return false;
  }

  gfx_reset_refresh_rate();

  return true;
}

qboolean gfx_set_vsync(qboolean vsync)
{
  if (renderer == NULL) {
    return false;
  }

#if SDL_VERSION_ATLEAST(2, 0, 18)
  if (SDL_RenderSetVSync(renderer, vsync ? 1 : 0) < 0) {
    Com_Printf("Failed to %s VSync: %s\n", vsync ? "enable" : "disable", SDL_GetError());
    return false;
  }

  return true;
#else
  Uint32 flags = SDL_RENDERER_ACCELERATED;

  if (vsync) {
    flags |= SDL_RENDERER_PRESENTVSYNC;
  }

  DestroyRenderTarget();
  SDL_DestroyRenderer(renderer);

  renderer = SDL_CreateRenderer(window, -1, flags);
  if (renderer == NULL) {
    Com_Printf("Failed to recreate renderer for VSync: %s\n", SDL_GetError());
    return false;
  }

  return gfx_resize_render_target(render_width, render_height);
#endif
}

qboolean gfx_set_refresh_rate(int refresh_rate, int width, int height)
{
  SDL_DisplayMode requested;
  SDL_DisplayMode closest;
  int display;

  if (window == NULL) {
    return false;
  }

  display = SDL_GetWindowDisplayIndex(window);
  if (display < 0) {
    Com_Printf("Failed to determine the window display: %s\n", SDL_GetError());
    return false;
  }

  memset(&requested, 0, sizeof(requested));
  requested.w = width;
  requested.h = height;
  requested.refresh_rate = refresh_rate;

  if (!SDL_GetClosestDisplayMode(display, &requested, &closest)) {
    Com_Printf("No display mode near %dx%d@%d\n", width, height, refresh_rate);
    return false;
  }

  if (SDL_SetWindowDisplayMode(window, &closest) < 0) {
    Com_Printf("Failed to select %dx%d@%d: %s\n", closest.w, closest.h, closest.refresh_rate, SDL_GetError());
    return false;
  }

  gfx_reset_refresh_rate();

  return true;
}

void gfx_update(swstate_t sw_state, viddef_t vid)
{
  const unsigned char *palette = sw_state.currentpalette;
  int i;

  if (surface_indexed == NULL || surface == NULL || texture == NULL) {
    return;
  }

  if (memcmp(cached_palette, palette, sizeof(cached_palette)) != 0) {
    SDL_Color colors[256];

    memcpy(cached_palette, palette, sizeof(cached_palette));

    for (i = 0; i < 256; i++) {
      colors[i].r = palette[i * 4 + 0];
      colors[i].g = palette[i * 4 + 1];
      colors[i].b = palette[i * 4 + 2];
      colors[i].a = 255;
    }

    SDL_SetPaletteColors(surface_indexed->format->palette, colors, 0, 256);
  }

  for (i = 0; i < vid.height; i++) {
    memcpy((Uint8 *) surface_indexed->pixels + i * surface_indexed->pitch, vid_buffer + i * vid.width, vid.width);
  }

  SDL_BlitSurface(surface_indexed, NULL, surface, NULL);

  SDL_UpdateTexture(texture, NULL, surface->pixels, surface->pitch);

  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);
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
  DestroyRenderTarget();

  if (renderer) {
    SDL_DestroyRenderer(renderer);
  }

  renderer = NULL;

  if (window) {
    SDL_DestroyWindow(window);
  }

  window = NULL;
  render_width = 0;
  render_height = 0;
  mouse_grabbed = false;
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
