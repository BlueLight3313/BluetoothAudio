#pragma once
#include <stdint.h>
#include <stddef.h>
typedef struct audio_io audio_io_t;
typedef enum { AUDIO_BACKEND_ALSA=1, AUDIO_BACKEND_OPENSLES=2 } audio_backend_t;
typedef struct { int sample_rate; int channels; int frames_per_buf; } audio_cfg_t;
audio_io_t* audio_open(audio_backend_t backend, const audio_cfg_t* cfg);
void        audio_close(audio_io_t* a);
int         audio_read_pcm16(audio_io_t* a, int16_t* out, size_t frames);
int         audio_write_pcm16(audio_io_t* a, const int16_t* in, size_t frames);
