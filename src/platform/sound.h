#ifndef PLATFORM_SOUND_H
#define PLATFORM_SOUND_H

#include <stdint.h>
#include "../common/header/common.h"
#include "../client/header/client.h"
#include "../client/sound/header/local.h"

qboolean sound_init(void);
void sound_shutdown(void);
void sound_print_debug_info(void);
int sound_set_playback_offset(float);
void sound_clear_buffer(void);
qboolean sound_cache(sfx_t *sfx, wavinfo_t *info, byte *data);
void sound_update(void);
void sound_raw_samples(int samples, int rate, int width, int channels, byte *data, float volume);
void sound_spatialize(channel_t *ch);

#endif // PLATFORM_SOUND_H
