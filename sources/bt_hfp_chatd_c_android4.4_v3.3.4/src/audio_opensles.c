#include "audio.h"
#ifdef __ANDROID__
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <stdlib.h>
#include <string.h>
struct audio_io {
  SLObjectItf engine_obj; SLEngineItf engine; SLObjectItf outmix_obj;
  SLObjectItf player_obj; SLPlayItf player; SLAndroidSimpleBufferQueueItf pbq;
  SLObjectItf recorder_obj; SLRecordItf recorder; SLAndroidSimpleBufferQueueItf rbq;
  audio_cfg_t cfg; int16_t* rec_buf; int16_t* play_buf; volatile int rec_ready; volatile int play_free;
};
static void rbq_cb(SLAndroidSimpleBufferQueueItf bq, void* ctx){ (void)bq; ((struct audio_io*)ctx)->rec_ready=1; }
static void pbq_cb(SLAndroidSimpleBufferQueueItf bq, void* ctx){ (void)bq; ((struct audio_io*)ctx)->play_free=1; }
static int init_engine(struct audio_io* a){
  if(slCreateEngine(&a->engine_obj,0,0,0,0,0)!=SL_RESULT_SUCCESS) return -1;
  if((*a->engine_obj)->Realize(a->engine_obj,SL_BOOLEAN_FALSE)!=SL_RESULT_SUCCESS) return -1;
  if((*a->engine_obj)->GetInterface(a->engine_obj,SL_IID_ENGINE,&a->engine)!=SL_RESULT_SUCCESS) return -1;
  if((*a->engine)->CreateOutputMix(a->engine,&a->outmix_obj,0,0,0)!=SL_RESULT_SUCCESS) return -1;
  if((*a->outmix_obj)->Realize(a->outmix_obj,SL_BOOLEAN_FALSE)!=SL_RESULT_SUCCESS) return -1;
  return 0;
}
static int init_player(struct audio_io* a){
  SLDataLocator_AndroidSimpleBufferQueue loc_bq={SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,2};
  SLDataFormat_PCM fmt={SL_DATAFORMAT_PCM,(SLuint32)a->cfg.channels,(SLuint32)(a->cfg.sample_rate*1000),
    SL_PCMSAMPLEFORMAT_FIXED_16,SL_PCMSAMPLEFORMAT_FIXED_16,
    (a->cfg.channels==1)?SL_SPEAKER_FRONT_CENTER:(SL_SPEAKER_FRONT_LEFT|SL_SPEAKER_FRONT_RIGHT),SL_BYTEORDER_LITTLEENDIAN};
  SLDataSource src={&loc_bq,&fmt};
  SLDataLocator_OutputMix loc_out={SL_DATALOCATOR_OUTPUTMIX,a->outmix_obj};
  SLDataSink snk={&loc_out,0};
  const SLInterfaceID ids[1]={SL_IID_ANDROIDSIMPLEBUFFERQUEUE}; const SLboolean req[1]={SL_BOOLEAN_TRUE};
  if((*a->engine)->CreateAudioPlayer(a->engine,&a->player_obj,&src,&snk,1,ids,req)!=SL_RESULT_SUCCESS) return -1;
  if((*a->player_obj)->Realize(a->player_obj,SL_BOOLEAN_FALSE)!=SL_RESULT_SUCCESS) return -1;
  if((*a->player_obj)->GetInterface(a->player_obj,SL_IID_PLAY,&a->player)!=SL_RESULT_SUCCESS) return -1;
  if((*a->player_obj)->GetInterface(a->player_obj,SL_IID_ANDROIDSIMPLEBUFFERQUEUE,&a->pbq)!=SL_RESULT_SUCCESS) return -1;
  if((*a->pbq)->RegisterCallback(a->pbq,pbq_cb,a)!=SL_RESULT_SUCCESS) return -1;
  a->play_free=1; (*a->player)->SetPlayState(a->player,SL_PLAYSTATE_PLAYING);
  return 0;
}
static int init_recorder(struct audio_io* a){
  SLDataLocator_IODevice loc_dev={SL_DATALOCATOR_IODEVICE,SL_IODEVICE_AUDIOINPUT,SL_DEFAULTDEVICEID_AUDIOINPUT,0};
  SLDataSource src={&loc_dev,0};
  SLDataLocator_AndroidSimpleBufferQueue loc_bq={SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,2};
  SLDataFormat_PCM fmt={SL_DATAFORMAT_PCM,(SLuint32)a->cfg.channels,(SLuint32)(a->cfg.sample_rate*1000),
    SL_PCMSAMPLEFORMAT_FIXED_16,SL_PCMSAMPLEFORMAT_FIXED_16,
    (a->cfg.channels==1)?SL_SPEAKER_FRONT_CENTER:(SL_SPEAKER_FRONT_LEFT|SL_SPEAKER_FRONT_RIGHT),SL_BYTEORDER_LITTLEENDIAN};
  SLDataSink snk={&loc_bq,&fmt};
  const SLInterfaceID ids[1]={SL_IID_ANDROIDSIMPLEBUFFERQUEUE}; const SLboolean req[1]={SL_BOOLEAN_TRUE};
  if((*a->engine)->CreateAudioRecorder(a->engine,&a->recorder_obj,&src,&snk,1,ids,req)!=SL_RESULT_SUCCESS) return -1;
  if((*a->recorder_obj)->Realize(a->recorder_obj,SL_BOOLEAN_FALSE)!=SL_RESULT_SUCCESS) return -1;
  if((*a->recorder_obj)->GetInterface(a->recorder_obj,SL_IID_RECORD,&a->recorder)!=SL_RESULT_SUCCESS) return -1;
  if((*a->recorder_obj)->GetInterface(a->recorder_obj,SL_IID_ANDROIDSIMPLEBUFFERQUEUE,&a->rbq)!=SL_RESULT_SUCCESS) return -1;
  if((*a->rbq)->RegisterCallback(a->rbq,rbq_cb,a)!=SL_RESULT_SUCCESS) return -1;
  a->rec_ready=0;
  (*a->rbq)->Enqueue(a->rbq,a->rec_buf,(SLuint32)(a->cfg.frames_per_buf*a->cfg.channels*sizeof(int16_t)));
  (*a->recorder)->SetRecordState(a->recorder,SL_RECORDSTATE_RECORDING);
  return 0;
}
audio_io_t* audio_open(audio_backend_t backend,const audio_cfg_t* cfg){
  if(backend!=AUDIO_BACKEND_OPENSLES) return NULL;
  struct audio_io* a=(struct audio_io*)calloc(1,sizeof(*a));
  a->cfg=*cfg;
  a->rec_buf=(int16_t*)calloc(cfg->frames_per_buf*cfg->channels,sizeof(int16_t));
  a->play_buf=(int16_t*)calloc(cfg->frames_per_buf*cfg->channels,sizeof(int16_t));
  if(init_engine(a)<0||init_player(a)<0||init_recorder(a)<0){ audio_close((audio_io_t*)a); return NULL; }
  return (audio_io_t*)a;
}
void audio_close(audio_io_t* a0){
  struct audio_io* a=(struct audio_io*)a0; if(!a) return;
  if(a->recorder_obj) (*a->recorder_obj)->Destroy(a->recorder_obj);
  if(a->player_obj) (*a->player_obj)->Destroy(a->player_obj);
  if(a->outmix_obj) (*a->outmix_obj)->Destroy(a->outmix_obj);
  if(a->engine_obj) (*a->engine_obj)->Destroy(a->engine_obj);
  free(a->rec_buf); free(a->play_buf); free(a);
}
int audio_read_pcm16(audio_io_t* a0,int16_t* out,size_t frames){
  struct audio_io* a=(struct audio_io*)a0; if(!a) return -1;
  while(!a->rec_ready){} a->rec_ready=0;
  memcpy(out,a->rec_buf,frames*a->cfg.channels*sizeof(int16_t));
  (*a->rbq)->Enqueue(a->rbq,a->rec_buf,(SLuint32)(frames*a->cfg.channels*sizeof(int16_t)));
  return (int)frames;
}
int audio_write_pcm16(audio_io_t* a0,const int16_t* in,size_t frames){
  struct audio_io* a=(struct audio_io*)a0; if(!a) return -1;
  while(!a->play_free){} a->play_free=0;
  memcpy(a->play_buf,in,frames*a->cfg.channels*sizeof(int16_t));
  (*a->pbq)->Enqueue(a->pbq,a->play_buf,(SLuint32)(frames*a->cfg.channels*sizeof(int16_t)));
  return (int)frames;
}
#else
struct audio_io{int dummy;};
audio_io_t* audio_open(audio_backend_t b,const audio_cfg_t* c){(void)b;(void)c;return NULL;}
void audio_close(audio_io_t* a){(void)a;}
int audio_read_pcm16(audio_io_t* a,int16_t* o,size_t f){(void)a;(void)o;(void)f;return -1;}
int audio_write_pcm16(audio_io_t* a,const int16_t* i,size_t f){(void)a;(void)i;(void)f;return -1;}
#endif
