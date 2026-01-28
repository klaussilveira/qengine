#include <SDL.h>
#include "../graphics.h"

static SDL_Window *window = NULL;
static SDL_Surface *surface = NULL;
static SDL_Texture *texture = NULL;
static SDL_Texture *texture_upscaled = NULL;
static SDL_Renderer *renderer = NULL;
static int refreshRate = -1;

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

static void CreateUpscaledTexture(void)
{
  int w, h;
  int w_upscale, h_upscale;

  if (renderer == NULL || render_width == 0 || render_height == 0) {
    return;
  }

  if (SDL_GetRendererOutputSize(renderer, &w, &h) != 0) {
    Com_Printf("Failed to get renderer output size: %s\n", SDL_GetError());
    return;
  }

  // Calculate integer scale factors
  w_upscale = (w + render_width - 1) / render_width;
  h_upscale = (h + render_height - 1) / render_height;

  if (w_upscale < 1) w_upscale = 1;
  if (h_upscale < 1) h_upscale = 1;

  // Check texture size limits
  SDL_RendererInfo info;
  SDL_GetRendererInfo(renderer, &info);

  while (w_upscale * render_width > info.max_texture_width) {
    w_upscale--;
  }
  while (h_upscale * render_height > info.max_texture_height) {
    h_upscale--;
  }

  if (texture_upscaled != NULL) {
    SDL_DestroyTexture(texture_upscaled);
  }

  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
  texture_upscaled = SDL_CreateTexture(renderer,
                                       SDL_PIXELFORMAT_ARGB8888,
                                       SDL_TEXTUREACCESS_TARGET,
                                       w_upscale * render_width,
                                       h_upscale * render_height);

  if (texture_upscaled == NULL) {
    Com_Printf("Failed to create upscaled texture: %s\n", SDL_GetError());
  } else {
    Com_Printf("Created upscaled texture: %dx%d (scale %dx%d)\n",
               w_upscale * render_width, h_upscale * render_height,
               w_upscale, h_upscale);
  }
}

qboolean gfx_create_window(qboolean fullscreen, qboolean vsync, int win_width, int win_height, int rend_width, int rend_height)
{
  Uint32 Rmask, Gmask, Bmask, Amask;
  Uint32 flags = SDL_SWSURFACE;
  int bpp;
  int windowPos = SDL_WINDOWPOS_CENTERED;

  if (!SDL_PixelFormatEnumToMasks(SDL_PIXELFORMAT_ARGB8888, &bpp, &Rmask, &Gmask, &Bmask, &Amask)) {
    return false;
  }

  // Store dimensions
  window_width = win_width;
  window_height = win_height;
  render_width = rend_width;
  render_height = rend_height;

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

  if (vsync) {
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
  } else {
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
  }

  if (renderer == NULL) {
    Com_Printf("Failed to create renderer: %s\n", SDL_GetError());
    return false;
  }

  // Surface and texture render resolution (not window)
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
  surface = SDL_CreateRGBSurface(0, render_width, render_height, bpp, Rmask, Gmask, Bmask, Amask);
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, render_width, render_height);

  // Create the upscaled texture for integer scaling
  CreateUpscaledTexture();

  SDL_ShowCursor(0);

  Com_Printf("Window: %dx%d, Render: %dx%d\n", win_width, win_height, render_width, render_height);

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
    return -1;
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

  if (window == NULL) {
    return false;
  }

  if (fullscreen == 1) {
    flags = SDL_WINDOW_FULLSCREEN_DESKTOP;
  } else if (fullscreen == 2) {
    flags = SDL_WINDOW_FULLSCREEN;
  }

  if (fullscreen != gfx_is_fullscreen()) {
    SDL_SetWindowFullscreen(window, flags);
    Cvar_SetValue("vid_fullscreen", fullscreen);
    CreateUpscaledTexture();

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

  // Convert 8-bit indexed to ARGB8888
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

  if (texture_upscaled != NULL) {
    // Two-pass rendering for crisp integer scaling
    SDL_SetRenderTarget(renderer, texture_upscaled);
    SDL_RenderCopy(renderer, texture, NULL, NULL);

    // Second pass
    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderCopy(renderer, texture_upscaled, NULL, NULL);
  } else {
    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
  }

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
  if (texture_upscaled) {
    SDL_DestroyTexture(texture_upscaled);
  }

  texture_upscaled = NULL;

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
