#include "sco_bridge.h"
#include "third_party/linux_uapi_bt/sco.h"
#include "bt_util.h"
#include "cvsd.h"
#include "audio.h"
#include "log.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

struct sco_bridge{
  int fd; bdaddr_t remote; pthread_t tx,rx; volatile int running;
  cvsd_t enc,dec; audio_io_t* audio; audio_cfg_t cfg;
};

static int open_sco(struct sco_bridge* b){
  bdaddr_t any; bt_bdaddr_any(&any);
  b->fd=socket(AF_BLUETOOTH,SOCK_SEQPACKET,BTPROTO_SCO);
  if(b->fd<0) return -1;
  struct sockaddr_sco loc; memset(&loc,0,sizeof(loc)); loc.sco_family=AF_BLUETOOTH; loc.sco_bdaddr=any;
  if(bind(b->fd,(struct sockaddr*)&loc,sizeof(loc))<0) return -1;
  struct sockaddr_sco rem; memset(&rem,0,sizeof(rem)); rem.sco_family=AF_BLUETOOTH; rem.sco_bdaddr=b->remote;
  if(connect(b->fd,(struct sockaddr*)&rem,sizeof(rem))<0) return -1;
  return 0;
}
static void* tx_th(void* p){
  struct sco_bridge* b=(struct sco_bridge*)p;
  int16_t* pcm=(int16_t*)calloc(b->cfg.frames_per_buf,sizeof(int16_t));
  uint8_t cv[160];
  while(b->running){
    int got=audio_read_pcm16(b->audio,pcm,b->cfg.frames_per_buf);
    if(got<=0) continue;
    size_t n=cvsd_encode(pcm,(size_t)got,cv,sizeof(cv),&b->enc);
    if(n&&send(b->fd,cv,n,0)<=0) break;
  }
  free(pcm); return 0;
}
static void* rx_th(void* p){
  struct sco_bridge* b=(struct sco_bridge*)p;
  uint8_t cv[240]; int16_t pcm[512];
  while(b->running){
    ssize_t n=recv(b->fd,cv,sizeof(cv),0);
    if(n<=0) break;
    size_t s=cvsd_decode(cv,(size_t)n,pcm,sizeof(pcm)/2,&b->dec);
    if(s) audio_write_pcm16(b->audio,pcm,s);
  }
  return 0;
}

sco_bridge_t* sco_bridge_start(const bdaddr_t* remote,int use_opensles){
  struct sco_bridge* b=(struct sco_bridge*)calloc(1,sizeof(*b));
  b->remote=*remote; b->cfg.sample_rate=8000; b->cfg.channels=1; b->cfg.frames_per_buf=160;
  audio_backend_t be=use_opensles?AUDIO_BACKEND_OPENSLES : AUDIO_BACKEND_LINUX_RAW;
  b->audio=audio_open(be,&b->cfg);
  if(!b->audio){ free(b); return NULL; }
  if(open_sco(b)<0){ audio_close(b->audio); free(b); return NULL; }
  cvsd_init(&b->enc); cvsd_init(&b->dec);
  b->running=1;
  pthread_create(&b->tx,0,tx_th,b);
  pthread_create(&b->rx,0,rx_th,b);
  LOGI("SCO started");
  return (sco_bridge_t*)b;
}
void sco_bridge_stop(sco_bridge_t* b0){
  struct sco_bridge* b=(struct sco_bridge*)b0; if(!b) return;
  b->running=0; shutdown(b->fd,SHUT_RDWR);
  pthread_join(b->tx,0); pthread_join(b->rx,0);
  close(b->fd); audio_close(b->audio); free(b);
}
