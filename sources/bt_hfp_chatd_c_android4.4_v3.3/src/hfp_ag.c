#include "hfp_ag.h"
#include "third_party/linux_uapi_bt/rfcomm.h"
#include "bt_util.h"
#include "log.h"
#include "sco_bridge.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#define RFCOMM_CH 8

struct hfp_ag{
  int listen_fd, cli_fd; pthread_t th; volatile int running;
  bdaddr_t peer; char peer_str[18]; char display[64];
  sco_bridge_t* sco; int use_opensles;
};

static void send_line(struct hfp_ag* a,const char* s){
  if(a->cli_fd<0) return;
  char buf[512]; int n=snprintf(buf,sizeof(buf),"%s\r\n",s);
  if(n>0) write(a->cli_fd,buf,(size_t)n);
}
static void send_ok(struct hfp_ag* a){ send_line(a,"OK"); }
static void send_ring_clip(struct hfp_ag* a){
  send_line(a,"RING");
  const char* num=a->display[0]?a->display:a->peer_str;
  char l[256]; snprintf(l,sizeof(l),"+CLIP: \"%s\",129",num);
  send_line(a,l);
}

static void* th_main(void* p){
  struct hfp_ag* a=(struct hfp_ag*)p;
  while(a->running){
    struct sockaddr_rc rem; socklen_t len=sizeof(rem);
    int fd=accept(a->listen_fd,(struct sockaddr*)&rem,&len);
    if(fd<0) continue;
    a->cli_fd=fd; a->peer=rem.rc_bdaddr; bt_format_mac(&a->peer,a->peer_str);
    LOGI("RFCOMM connected %s",a->peer_str);

    char buf[1024]; size_t used=0;
    while(a->running){
      ssize_t n=read(fd,buf+used,sizeof(buf)-1-used);
      if(n<=0) break;
      used+=(size_t)n; buf[used]=0;
      char* cr;
      while((cr=strchr(buf,'\r'))){
        *cr=0;
        char line[256]; snprintf(line,sizeof(line),"%s",buf);
        size_t remn=used-((cr-buf)+1);
        memmove(buf,cr+1,remn); used=remn; buf[used]=0;
        if(line[0]==0) continue;

        if(strcmp(line,"AT")==0){ send_ok(a); continue; }
        if(strncmp(line,"AT+BRSF=",8)==0){ send_line(a,"+BRSF: 0"); send_ok(a); continue; }
        if(strcmp(line,"AT+CIND=?")==0){
          send_line(a,"+CIND: (\"service\",(0,1)),(\"call\",(0,1)),(\"callsetup\",(0-3)),(\"callheld\",(0-2))");
          send_ok(a); continue;
        }
        if(strcmp(line,"AT+CIND?")==0){ send_line(a,"+CIND: 1,0,0,0"); send_ok(a); continue; }
        if(strncmp(line,"AT+CMER=",8)==0){ send_ok(a); continue; }

        if(strcmp(line,"ATA")==0){
          send_ok(a);
          if(!a->sco) a->sco=sco_bridge_start(&a->peer,a->use_opensles);
          continue;
        }
        if(strcmp(line,"AT+CHUP")==0){
          send_ok(a);
          if(a->sco){ sco_bridge_stop(a->sco); a->sco=0; }
          continue;
        }

        if(strncmp(line,"ATD",3)==0){
          send_ok(a);
          send_ring_clip(a);
          continue;
        }

        if(strncmp(line,"AT+BAC=",7)==0){ send_ok(a); send_line(a,"AT+BCS=1"); continue; }
        if(strncmp(line,"AT+BCS=",7)==0){ send_ok(a); continue; }

        send_ok(a);
      }
    }
    LOGI("RFCOMM disconnected");
    close(fd); a->cli_fd=-1;
    if(a->sco){ sco_bridge_stop(a->sco); a->sco=0; }
  }
  return 0;
}

hfp_ag_t* hfp_ag_start(int use_opensles){
  struct hfp_ag* a=(struct hfp_ag*)calloc(1,sizeof(*a));
  a->listen_fd=socket(AF_BLUETOOTH,SOCK_STREAM,BTPROTO_RFCOMM);
  if(a->listen_fd<0){ free(a); return NULL; }
  struct sockaddr_rc loc; memset(&loc,0,sizeof(loc));
  loc.rc_family=AF_BLUETOOTH; bt_bdaddr_any(&loc.rc_bdaddr); loc.rc_channel=RFCOMM_CH;
  if(bind(a->listen_fd,(struct sockaddr*)&loc,sizeof(loc))<0){ close(a->listen_fd); free(a); return NULL; }
  if(listen(a->listen_fd,1)<0){ close(a->listen_fd); free(a); return NULL; }
  a->cli_fd=-1; a->running=1; a->use_opensles=use_opensles; a->display[0]=0; a->peer_str[0]=0;
  pthread_create(&a->th,0,th_main,a);
  LOGI("HFP AG started ch %d",RFCOMM_CH);
  return (hfp_ag_t*)a;
}
void hfp_ag_stop(hfp_ag_t* a0){
  struct hfp_ag* a=(struct hfp_ag*)a0; if(!a) return;
  a->running=0; shutdown(a->listen_fd,SHUT_RDWR);
  pthread_join(a->th,0);
  close(a->listen_fd);
  if(a->cli_fd>=0) close(a->cli_fd);
  if(a->sco) sco_bridge_stop(a->sco);
  free(a);
}
void hfp_ag_set_number(hfp_ag_t* a0,const char* s){
  struct hfp_ag* a=(struct hfp_ag*)a0; if(!a) return;
  if(!s) s="";
  snprintf(a->display,sizeof(a->display),"%s",s);
}
int hfp_ag_get_peer_mac(hfp_ag_t* a0,char out[18]){
  struct hfp_ag* a=(struct hfp_ag*)a0; if(!a||!out) return -1;
  snprintf(out,18,"%s",a->peer_str); return 0;
}
int hfp_ag_dial(hfp_ag_t* a0,const char* mac_str){
  struct hfp_ag* a=(struct hfp_ag*)a0;
  bdaddr_t b; if(bt_parse_mac(mac_str,&b)<0) return -1;
  a->peer=b; bt_format_mac(&a->peer,a->peer_str);
  if(!a->sco) a->sco=sco_bridge_start(&a->peer,a->use_opensles);
  return a->sco?0:-1;
}
int hfp_ag_hangup(hfp_ag_t* a0){
  struct hfp_ag* a=(struct hfp_ag*)a0;
  if(a && a->sco){ sco_bridge_stop(a->sco); a->sco=0; }
  return 0;
}
