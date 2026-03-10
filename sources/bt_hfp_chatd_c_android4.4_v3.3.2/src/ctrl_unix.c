#include "ctrl_server.h"
#include "log.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

struct ctrl_server{ int fd; pthread_t th; volatile int running; ctrl_handler_fn fn; void* user; char path[108]; };

static void* th_main(void* p){
  struct ctrl_server* s=(struct ctrl_server*)p;
  while(s->running){
    int cfd=accept(s->fd,0,0); if(cfd<0) continue;
    const char* hello="bt_chatd ready\n"; write(cfd,hello,strlen(hello));
    char buf[1024]; size_t used=0;
    while(s->running){
      ssize_t n=read(cfd,buf+used,sizeof(buf)-1-used); if(n<=0) break;
      used+=(size_t)n; buf[used]=0;
      char* nl;
      while((nl=strchr(buf,'\n'))){
        *nl=0; char line[512]; snprintf(line,sizeof(line),"%s",buf);
        size_t remn=used-((nl-buf)+1); memmove(buf,nl+1,remn); used=remn; buf[used]=0;

        size_t L=strlen(line); while(L && (line[L-1]=='\r'||line[L-1]==' '||line[L-1]=='\t')) line[--L]=0;
        char* p2=line; while(*p2==' '||*p2=='\t') p2++; if(*p2==0) continue;

        char out[1024];
        if(s->fn(p2,out,(int)sizeof(out),s->user)<0) snprintf(out,sizeof(out),"ERR");
        size_t olen=strlen(out); out[olen++]='\n';
        write(cfd,out,olen);
      }
    }
    close(cfd);
  }
  return 0;
}

ctrl_server_t* ctrl_server_unix_start(const char* path,ctrl_handler_fn fn,void* user){
  struct ctrl_server* s=(struct ctrl_server*)calloc(1,sizeof(*s)); s->fn=fn; s->user=user;
  snprintf(s->path,sizeof(s->path),"%s",path);
  s->fd=socket(AF_UNIX,SOCK_STREAM,0); if(s->fd<0){ free(s); return NULL; }
  unlink(path);
  struct sockaddr_un a; memset(&a,0,sizeof(a)); a.sun_family=AF_UNIX; snprintf(a.sun_path,sizeof(a.sun_path),"%s",path);
  if(bind(s->fd,(struct sockaddr*)&a,sizeof(a))<0){ close(s->fd); free(s); return NULL; }
  if(listen(s->fd,5)<0){ close(s->fd); free(s); return NULL; }
  s->running=1; pthread_create(&s->th,0,th_main,s);
  LOGI("UNIX control %s",path);
  return (ctrl_server_t*)s;
}

void ctrl_server_stop(ctrl_server_t* s0){
  struct ctrl_server* s=(struct ctrl_server*)s0; if(!s) return;
  s->running=0; shutdown(s->fd,SHUT_RDWR);
  pthread_join(s->th,0);
  close(s->fd);
  unlink(s->path);
  free(s);
}
