#include <SDL.h>
#include "../graphics.h"

static SDL_Window *window = NULL;
static SDL_Surface *surface = NULL;
static SDL_Texture *texture = NULL;
static SDL_Renderer *renderer = NULL;
static int refreshRate = -1;

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

qboolean gfx_create_window(qboolean fullscreen, qboolean vsync, int width, int height)
{
  Uint32 Rmask, Gmask, Bmask, Amask;
  Uint32 flags = SDL_SWSURFACE;
  int bpp;
  int windowPos = SDL_WINDOWPOS_CENTERED;

  if (!SDL_PixelFormatEnumToMasks(SDL_PIXELFORMAT_ARGB8888, &bpp, &Rmask, &Gmask, &Bmask, &Amask)) {
    return false;
  }

  if (fullscreen == 1) {
    flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
  } else if (fullscreen == 2) {
    flags |= SDL_WINDOW_FULLSCREEN;
  }

  window = SDL_CreateWindow("qengine", windowPos, windowPos, width, height, flags);

  if (vsync) {
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  } else {
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  }

  surface = SDL_CreateRGBSurface(0, width, height, bpp, Rmask, Gmask, Bmask, Amask);
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
  SDL_ShowCursor(0);

  return window != NULL;
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

void gfx_update(swstate_t sw_state, viddef_t vid)
{

  int y, x, i;
  const unsigned char *pallete = sw_state.currentpalette;
  Uint32 pallete_colors[256];

  for (i = 0; i < 256; i++) {
    pallete_colors[i] = SDL_MapRGB(surface->format, pallete[i * 4 + 0], // red
                                   pallete[i * 4 + 1],                  // green
                                   pallete[i * 4 + 2]                   // blue
    );
  }

  Uint32 *pixels = (Uint32 *) surface->pixels;
  for (y = 0; y < vid.height; y++) {
    for (x = 0; x < vid.width; x++) {
      int buffer_pos = y * vid.width + x;
      Uint32 color = pallete_colors[vid_buffer[buffer_pos]];
      pixels[y * surface->pitch / sizeof(Uint32) + x] = color;
    }
  }

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
  if (texture) {
    SDL_DestroyTexture(texture);
  }

  texture = NULL;

  if (surface) {
    SDL_FreeSurface(surface);
  }

  surface = NULL;

  if (renderer) {
    SDL_DestroyRenderer(renderer);
  }

  renderer = NULL;

  if (window) {
    SDL_DestroyWindow(window);
  }

  window = NULL;
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
{}
