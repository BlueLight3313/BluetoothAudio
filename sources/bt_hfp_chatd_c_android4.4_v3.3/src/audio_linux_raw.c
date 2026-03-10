#include "audio.h"
#include "log.h"

#ifndef __ANDROID__
// Raw kernel ALSA PCM interface (no libasound). Uses /dev/snd/pcmC*D*{c,p}.
#include <sound/asound.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

struct audio_io {
    int fd_cap;
    int fd_pb;
    audio_cfg_t cfg;
    size_t bytes_per_frame;
};

static int set_hw_params(int fd, const audio_cfg_t* cfg, int is_capture){
    struct snd_pcm_hw_params params;
    memset(&params, 0, sizeof(params));

    if(ioctl(fd, SNDRV_PCM_IOCTL_HW_REFINE, &params) < 0) {
        LOGE("HW_REFINE failed (%s): %s", is_capture ? "cap":"pb", strerror(errno));
        return -1;
    }

    params.access   = SNDRV_PCM_ACCESS_RW_INTERLEAVED;
    params.format   = SNDRV_PCM_FORMAT_S16_LE;
    params.channels = (unsigned int)cfg->channels;

    params.rate     = (unsigned int)cfg->sample_rate;
    params.rate_num = (unsigned int)cfg->sample_rate;
    params.rate_den = 1;

    params.period_size = (snd_pcm_uframes_t)cfg->frames_per_buf;

    if(ioctl(fd, SNDRV_PCM_IOCTL_HW_PARAMS, &params) < 0) {
        LOGE("HW_PARAMS failed (%s): %s", is_capture ? "cap":"pb", strerror(errno));
        return -1;
    }

    if(ioctl(fd, SNDRV_PCM_IOCTL_PREPARE) < 0) {
        LOGE("PREPARE failed (%s): %s", is_capture ? "cap":"pb", strerror(errno));
        return -1;
    }
    return 0;
}

audio_io_t* audio_open(audio_backend_t backend, const audio_cfg_t* cfg){
    if(backend != AUDIO_BACKEND_LINUX_RAW) return NULL;
    if(!cfg || !cfg->dev_capture || !cfg->dev_playback) return NULL;

    struct audio_io* a=(struct audio_io*)calloc(1,sizeof(*a));
    a->cfg = *cfg;
    a->bytes_per_frame = (size_t)(cfg->channels * (int)sizeof(int16_t));

    a->fd_cap = open(cfg->dev_capture, O_RDONLY);
    if(a->fd_cap < 0){
        LOGE("open capture %s failed: %s", cfg->dev_capture, strerror(errno));
        free(a);
        return NULL;
    }
    a->fd_pb = open(cfg->dev_playback, O_WRONLY);
    if(a->fd_pb < 0){
        LOGE("open playback %s failed: %s", cfg->dev_playback, strerror(errno));
        close(a->fd_cap);
        free(a);
        return NULL;
    }

    if(set_hw_params(a->fd_cap, cfg, 1) < 0){ audio_close((audio_io_t*)a); return NULL; }
    if(set_hw_params(a->fd_pb,  cfg, 0) < 0){ audio_close((audio_io_t*)a); return NULL; }

    return (audio_io_t*)a;
}

void audio_close(audio_io_t* a0){
    struct audio_io* a=(struct audio_io*)a0;
    if(!a) return;
    if(a->fd_cap>=0) close(a->fd_cap);
    if(a->fd_pb>=0) close(a->fd_pb);
    free(a);
}

static int read_full(int fd, void* buf, size_t n){
    uint8_t* p=(uint8_t*)buf;
    size_t got=0;
    while(got<n){
        ssize_t r=read(fd, p+got, n-got);
        if(r==0) return (int)got;
        if(r<0){
            if(errno==EINTR) continue;
            return -1;
        }
        got += (size_t)r;
    }
    return (int)got;
}

static int write_full(int fd, const void* buf, size_t n){
    const uint8_t* p=(const uint8_t*)buf;
    size_t sent=0;
    while(sent<n){
        ssize_t w=write(fd, p+sent, n-sent);
        if(w<=0){
            if(w<0 && errno==EINTR) continue;
            return -1;
        }
        sent += (size_t)w;
    }
    return (int)sent;
}

int audio_read_pcm16(audio_io_t* a0, int16_t* out, size_t frames){
    struct audio_io* a=(struct audio_io*)a0;
    if(!a||!out) return -1;
    size_t bytes = frames * a->bytes_per_frame;
    int r = read_full(a->fd_cap, out, bytes);
    if(r < 0) return -1;
    return (int)(r / (int)a->bytes_per_frame);
}

int audio_write_pcm16(audio_io_t* a0, const int16_t* in, size_t frames){
    struct audio_io* a=(struct audio_io*)a0;
    if(!a||!in) return -1;
    size_t bytes = frames * a->bytes_per_frame;
    int w = write_full(a->fd_pb, in, bytes);
    if(w < 0) return -1;
    return (int)(w / (int)a->bytes_per_frame);
}

#else
struct audio_io{int dummy;};
audio_io_t* audio_open(audio_backend_t backend, const audio_cfg_t* cfg){ (void)backend;(void)cfg; return NULL; }
void audio_close(audio_io_t* a){ (void)a; }
int audio_read_pcm16(audio_io_t* a, int16_t* out, size_t frames){ (void)a;(void)out;(void)frames; return -1; }
int audio_write_pcm16(audio_io_t* a, const int16_t* in, size_t frames){ (void)a;(void)in;(void)frames; return -1; }
#endif
