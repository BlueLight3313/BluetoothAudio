#include "audio.h"
#include <alsa/asoundlib.h>

static snd_pcm_t *capture, *playback;

void audio_init() {
    snd_pcm_open(&capture, "default", SND_PCM_STREAM_CAPTURE, 0);
    snd_pcm_open(&playback, "default", SND_PCM_STREAM_PLAYBACK, 0);
}

int audio_capture(char *buf, int size) {
    return snd_pcm_readi(capture, buf, size/2);
}

void audio_play(char *buf, int size) {
    snd_pcm_writei(playback, buf, size/2);
}