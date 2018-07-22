#include <SDL.h>
#include "../input.h"

#define MOUSE_MAX 3000
#define MOUSE_MIN 40

/* Globals */
static int mouse_x, mouse_y;
static int old_mouse_x, old_mouse_y;
static qboolean mlooking;

/* CVars */
cvar_t *vid_fullscreen;
static cvar_t *in_grab;
static cvar_t *exponential_speedup;
cvar_t *freelook;
cvar_t *lookstrafe;
cvar_t *m_forward;
static cvar_t *m_filter;
cvar_t *m_pitch;
cvar_t *m_side;
cvar_t *m_up;
cvar_t *m_yaw;
cvar_t *sensitivity;
static cvar_t *windowed_mouse;

static int input_translate_key_event(unsigned int keysym)
{
  int key = 0;

  /* These must be translated */
  switch (keysym) {
  case SDLK_PAGEUP:
    key = K_PGUP;
    break;
  case SDLK_KP_9:
    key = K_KP_PGUP;
    break;
  case SDLK_PAGEDOWN:
    key = K_PGDN;
    break;
  case SDLK_KP_3:
    key = K_KP_PGDN;
    break;
  case SDLK_KP_7:
    key = K_KP_HOME;
    break;
  case SDLK_HOME:
    key = K_HOME;
    break;
  case SDLK_KP_1:
    key = K_KP_END;
    break;
  case SDLK_END:
    key = K_END;
    break;
  case SDLK_KP_4:
    key = K_KP_LEFTARROW;
    break;
  case SDLK_LEFT:
    key = K_LEFTARROW;
    break;
  case SDLK_KP_6:
    key = K_KP_RIGHTARROW;
    break;
  case SDLK_RIGHT:
    key = K_RIGHTARROW;
    break;
  case SDLK_KP_2:
    key = K_KP_DOWNARROW;
    break;
  case SDLK_DOWN:
    key = K_DOWNARROW;
    break;
  case SDLK_KP_8:
    key = K_KP_UPARROW;
    break;
  case SDLK_UP:
    key = K_UPARROW;
    break;
  case SDLK_ESCAPE:
    key = K_ESCAPE;
    break;
  case SDLK_KP_ENTER:
    key = K_KP_ENTER;
    break;
  case SDLK_RETURN:
    key = K_ENTER;
    break;
  case SDLK_TAB:
    key = K_TAB;
    break;
  case SDLK_F1:
    key = K_F1;
    break;
  case SDLK_F2:
    key = K_F2;
    break;
  case SDLK_F3:
    key = K_F3;
    break;
  case SDLK_F4:
    key = K_F4;
    break;
  case SDLK_F5:
    key = K_F5;
    break;
  case SDLK_F6:
    key = K_F6;
    break;
  case SDLK_F7:
    key = K_F7;
    break;
  case SDLK_F8:
    key = K_F8;
    break;
  case SDLK_F9:
    key = K_F9;
    break;
  case SDLK_F10:
    key = K_F10;
    break;
  case SDLK_F11:
    key = K_F11;
    break;
  case SDLK_F12:
    key = K_F12;
    break;
  case SDLK_F13:
    key = K_F13;
    break;
  case SDLK_F14:
    key = K_F14;
    break;
  case SDLK_F15:
    key = K_F15;
    break;
  case SDLK_BACKSPACE:
    key = K_BACKSPACE;
    break;
  case SDLK_KP_PERIOD:
    key = K_KP_DEL;
    break;
  case SDLK_DELETE:
    key = K_DEL;
    break;
  case SDLK_PAUSE:
    key = K_PAUSE;
    break;
  case SDLK_LSHIFT:
  case SDLK_RSHIFT:
    key = K_SHIFT;
    break;
  case SDLK_LCTRL:
  case SDLK_RCTRL:
    key = K_CTRL;
    break;
  case SDLK_RGUI:
  case SDLK_LGUI:
    key = K_COMMAND;
    break;
  case SDLK_RALT:
  case SDLK_LALT:
    key = K_ALT;
    break;
  case SDLK_KP_5:
    key = K_KP_5;
    break;
  case SDLK_INSERT:
    key = K_INS;
    break;
  case SDLK_KP_0:
    key = K_KP_INS;
    break;
  case SDLK_KP_MULTIPLY:
    key = K_KP_STAR;
    break;
  case SDLK_KP_PLUS:
    key = K_KP_PLUS;
    break;
  case SDLK_KP_MINUS:
    key = K_KP_MINUS;
    break;
  case SDLK_KP_DIVIDE:
    key = K_KP_SLASH;
    break;
  case SDLK_MODE:
    key = K_MODE;
    break;
  case SDLK_APPLICATION:
    key = K_COMPOSE;
    break;
  case SDLK_HELP:
    key = K_HELP;
    break;
  case SDLK_PRINTSCREEN:
    key = K_PRINT;
    break;
  case SDLK_SYSREQ:
    key = K_SYSREQ;
    break;
  case SDLK_MENU:
    key = K_MENU;
    break;
  case SDLK_POWER:
    key = K_POWER;
    break;
  case SDLK_UNDO:
    key = K_UNDO;
    break;
  case SDLK_SCROLLLOCK:
    key = K_SCROLLOCK;
    break;
  case SDLK_NUMLOCKCLEAR:
    key = K_KP_NUMLOCK;
    break;
  case SDLK_CAPSLOCK:
    key = K_CAPSLOCK;
    break;

  default:
    break;
  }

  return key;
}

static void on_mouse_look_down(void)
{
  mlooking = true;
}

static void on_mouse_look_up(void)
{
  mlooking = false;
  IN_CenterView();
}

void input_init()
{
  mouse_x = mouse_y = 0;

  exponential_speedup = Cvar_Get("exponential_speedup", "0", CVAR_ARCHIVE);
  freelook = Cvar_Get("freelook", "1", 0);
  in_grab = Cvar_Get("in_grab", "2", CVAR_ARCHIVE);
  lookstrafe = Cvar_Get("lookstrafe", "0", 0);
  m_filter = Cvar_Get("m_filter", "0", CVAR_ARCHIVE);
  m_up = Cvar_Get("m_up", "1", 0);
  m_forward = Cvar_Get("m_forward", "1", 0);
  m_pitch = Cvar_Get("m_pitch", "0.022", 0);
  m_side = Cvar_Get("m_side", "0.8", 0);
  m_yaw = Cvar_Get("m_yaw", "0.022", 0);
  sensitivity = Cvar_Get("sensitivity", "3", 0);

  vid_fullscreen = Cvar_Get("vid_fullscreen", "0", CVAR_ARCHIVE);
  windowed_mouse = Cvar_Get("windowed_mouse", "1", CVAR_USERINFO | CVAR_ARCHIVE);

  Cmd_AddCommand("+mlook", on_mouse_look_down);
  Cmd_AddCommand("-mlook", on_mouse_look_up);

  SDL_StartTextInput();
}

void input_command(usercmd_t *cmd)
{
  if (m_filter->value) {
    if ((mouse_x > 1) || (mouse_x < -1)) {
      mouse_x = (mouse_x + old_mouse_x) * 0.5;
    }

    if ((mouse_y > 1) || (mouse_y < -1)) {
      mouse_y = (mouse_y + old_mouse_y) * 0.5;
    }
  }

  old_mouse_x = mouse_x;
  old_mouse_y = mouse_y;

  if (mouse_x || mouse_y) {
    if (!exponential_speedup->value) {
      mouse_x *= sensitivity->value;
      mouse_y *= sensitivity->value;
    } else {
      if ((mouse_x > MOUSE_MIN) || (mouse_y > MOUSE_MIN) || (mouse_x < -MOUSE_MIN) || (mouse_y < -MOUSE_MIN)) {
        mouse_x = (mouse_x * mouse_x * mouse_x) / 4;
        mouse_y = (mouse_y * mouse_y * mouse_y) / 4;

        if (mouse_x > MOUSE_MAX) {
          mouse_x = MOUSE_MAX;
        } else if (mouse_x < -MOUSE_MAX) {
          mouse_x = -MOUSE_MAX;
        }

        if (mouse_y > MOUSE_MAX) {
          mouse_y = MOUSE_MAX;
        } else if (mouse_y < -MOUSE_MAX) {
          mouse_y = -MOUSE_MAX;
        }
      }
    }

    /* add mouse X/Y movement to cmd */
    if ((in_strafe.state & 1) || (lookstrafe->value && mlooking)) {
      cmd->sidemove += m_side->value * mouse_x;
    } else {
      cl.viewangles[YAW] -= m_yaw->value * mouse_x;
    }

    if ((mlooking || freelook->value) && !(in_strafe.state & 1)) {
      cl.viewangles[PITCH] += m_pitch->value * mouse_y;
    } else {
      cmd->forwardmove -= m_forward->value * mouse_y;
    }

    mouse_x = mouse_y = 0;
  }
}

void input_shutdown()
{
  Cmd_RemoveCommand("force_centerview");
  Cmd_RemoveCommand("+mlook");
  Cmd_RemoveCommand("-mlook");

  Com_Printf("Shutting down input.\n");
}

void input_update()
{
  qboolean want_grab;
  SDL_Event event;
  unsigned int key;

  /* Get and process an event */
  while (SDL_PollEvent(&event)) {

    switch (event.type) {
    case SDL_MOUSEWHEEL:
      Key_Event((event.wheel.y > 0 ? K_MWHEELUP : K_MWHEELDOWN), true, true);
      Key_Event((event.wheel.y > 0 ? K_MWHEELUP : K_MWHEELDOWN), false, true);
      break;
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
      switch (event.button.button) {
      case SDL_BUTTON_LEFT:
        key = K_MOUSE1;
        break;
      case SDL_BUTTON_MIDDLE:
        key = K_MOUSE3;
        break;
      case SDL_BUTTON_RIGHT:
        key = K_MOUSE2;
        break;
      case SDL_BUTTON_X1:
        key = K_MOUSE4;
        break;
      case SDL_BUTTON_X2:
        key = K_MOUSE5;
        break;
      default:
        return;
      }

      Key_Event(key, (event.type == SDL_MOUSEBUTTONDOWN), true);
      break;

    case SDL_MOUSEMOTION:
      if (cls.key_dest == key_game && (int) cl_paused->value == 0) {
        mouse_x += event.motion.xrel;
        mouse_y += event.motion.yrel;
      }
      break;

    case SDL_TEXTINPUT:
      if ((event.text.text[0] >= ' ') && (event.text.text[0] <= '~')) {
        Char_Event(event.text.text[0]);
      }

      break;

    case SDL_KEYDOWN:
    case SDL_KEYUP: {
      qboolean down = (event.type == SDL_KEYDOWN);

      /* workaround for AZERTY-keyboards, which don't have 1, 2, ..., 9, 0 in
       * first row: always map those physical keys (scancodes) to those keycodes
       * anyway see also https://bugzilla.libsdl.org/show_bug.cgi?id=3188 */
      SDL_Scancode sc = event.key.keysym.scancode;
      if (sc >= SDL_SCANCODE_1 && sc <= SDL_SCANCODE_0) {
        /* Note that the SDL_SCANCODEs are SDL_SCANCODE_1, _2, ..., _9,
         * SDL_SCANCODE_0 while in ASCII it's '0', '1', ..., '9' => handle 0 and
         * 1-9 separately (quake2 uses the ASCII values for those keys) */
        int key = '0'; /* implicitly handles SDL_SCANCODE_0 */
        if (sc <= SDL_SCANCODE_9) {
          key = '1' + (sc - SDL_SCANCODE_1);
        }
        Key_Event(key, down, false);
      } else if ((event.key.keysym.sym >= SDLK_SPACE) && (event.key.keysym.sym < SDLK_DELETE)) {
        Key_Event(event.key.keysym.sym, down, false);
      } else {
        Key_Event(input_translate_key_event(event.key.keysym.sym), down, true);
      }
    } break;

    case SDL_WINDOWEVENT:
      if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST || event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
        Key_MarkAllUp();
      } else if (event.window.event == SDL_WINDOWEVENT_MOVED) {
        gfx_reset_refresh_rate();
      }

      break;

    case SDL_QUIT:
      Com_Quit();

      break;
    }
  }

  /* Grab and ungrab the mouse if the* console or the menu is opened */
  want_grab = (vid_fullscreen->value || in_grab->value == 1 || (in_grab->value == 2 && windowed_mouse->value));
  gfx_window_grab_input(want_grab);
}
