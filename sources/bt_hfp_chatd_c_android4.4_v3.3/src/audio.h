#pragma once
#include <stdint.h>
#include <stddef.h>

typedef struct audio_io audio_io_t;

typedef enum {
    AUDIO_BACKEND_LINUX_RAW = 1,
    AUDIO_BACKEND_OPENSLES  = 2
} audio_backend_t;

typedef struct {
    int sample_rate;        // 8000 for CVSD
    int channels;           // 1
    int frames_per_buf;     // 160 typical
    const char* dev_capture;   // linuxraw only, e.g. /dev/snd/pcmC0D0c
    const char* dev_playback;  // linuxraw only, e.g. /dev/snd/pcmC0D0p
} audio_cfg_t;

audio_io_t* audio_open(audio_backend_t backend, const audio_cfg_t* cfg);
void        audio_close(audio_io_t* a);
int         audio_read_pcm16(audio_io_t* a, int16_t* out, size_t frames);
int         audio_write_pcm16(audio_io_t* a, const int16_t* in, size_t frames);
