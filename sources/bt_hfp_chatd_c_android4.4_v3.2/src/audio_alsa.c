#include "audio.h"
#ifndef __ANDROID__
#include <alsa/asoundlib.h>
#include <stdlib.h>
struct audio_io { snd_pcm_t* cap; snd_pcm_t* pb; audio_cfg_t cfg; };
static int setup(snd_pcm_t* h,const audio_cfg_t* c){
    snd_pcm_hw_params_t* hw; snd_pcm_hw_params_alloca(&hw);
    if(snd_pcm_hw_params_any(h,hw)<0) return -1;
    if(snd_pcm_hw_params_set_access(h,hw,SND_PCM_ACCESS_RW_INTERLEAVED)<0) return -1;
    if(snd_pcm_hw_params_set_format(h,hw,SND_PCM_FORMAT_S16_LE)<0) return -1;
    unsigned int rate=(unsigned)c->sample_rate;
    if(snd_pcm_hw_params_set_rate_near(h,hw,&rate,0)<0) return -1;
    if(snd_pcm_hw_params_set_channels(h,hw,(unsigned)c->channels)<0) return -1;
    snd_pcm_uframes_t period=(snd_pcm_uframes_t)c->frames_per_buf;
    snd_pcm_hw_params_set_period_size_near(h,hw,&period,0);
    if(snd_pcm_hw_params(h,hw)<0) return -1;
    snd_pcm_prepare(h); return 0;
}
audio_io_t* audio_open(audio_backend_t backend,const audio_cfg_t* cfg){
    if(backend!=AUDIO_BACKEND_ALSA) return NULL;
    struct audio_io* a=(struct audio_io*)calloc(1,sizeof(*a)); a->cfg=*cfg;
    if(snd_pcm_open(&a->cap,"default",SND_PCM_STREAM_CAPTURE,0)<0){ free(a); return NULL; }
    if(snd_pcm_open(&a->pb,"default",SND_PCM_STREAM_PLAYBACK,0)<0){ snd_pcm_close(a->cap); free(a); return NULL; }
    if(setup(a->cap,cfg)<0||setup(a->pb,cfg)<0){ audio_close((audio_io_t*)a); return NULL; }
    return (audio_io_t*)a;
}
void audio_close(audio_io_t* a0){
    struct audio_io* a=(struct audio_io*)a0; if(!a) return;
    if(a->cap) snd_pcm_close(a->cap);
    if(a->pb) snd_pcm_close(a->pb);
    free(a);
}
int audio_read_pcm16(audio_io_t* a0,int16_t* out,size_t frames){
    struct audio_io* a=(struct audio_io*)a0;
    snd_pcm_sframes_t r=snd_pcm_readi(a->cap,out,frames);
    if(r<0){ snd_pcm_recover(a->cap,(int)r,1); return 0; }
    return (int)r;
}
int audio_write_pcm16(audio_io_t* a0,const int16_t* in,size_t frames){
    struct audio_io* a=(struct audio_io*)a0;
    snd_pcm_sframes_t w=snd_pcm_writei(a->pb,in,frames);
    if(w<0){ snd_pcm_recover(a->pb,(int)w,1); return 0; }
    return (int)w;
}
#else
struct audio_io{int dummy;};
audio_io_t* audio_open(audio_backend_t b,const audio_cfg_t* c){(void)b;(void)c;return NULL;}
void audio_close(audio_io_t* a){(void)a;}
int audio_read_pcm16(audio_io_t* a,int16_t* o,size_t f){(void)a;(void)o;(void)f;return -1;}
int audio_write_pcm16(audio_io_t* a,const int16_t* i,size_t f){(void)a;(void)i;(void)f;return -1;}
#endif
