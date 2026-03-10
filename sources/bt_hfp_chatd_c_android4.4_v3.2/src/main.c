#include "log.h"
#include "ctrl_server.h"
#include "hfp_ag.h"
#include "bt_mgmt.h"
#include "bt_util.h"
#include "state_machine.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct {
  hfp_ag_t* hfp;
  int use_opensles;
  app_state_t st;
  state_machine_t* sm;
} app_t;

static int on_cmd(const char* line,char* out,int out_sz,void* user){
  app_t* a=(app_t*)user;

  if(strcmp(line,"STATE")==0){
    char tgt[18]="(none)";
    char con[18]="(none)";
    if(a->st.has_target) bt_format_mac(&a->st.target, tgt);
    if(a->st.connected) bt_format_mac(&a->st.connected_addr, con);
    snprintf(out,out_sz,
      "auto=%d target=%s powered=%d scanning=%d connected=%d conn=%s rfcomm=%d sco=%d scan=[%s]",
      a->st.auto_enabled, tgt, a->st.powered, a->st.scanning, a->st.connected, con,
      a->st.rfcomm_connected, a->st.sco_running, a->st.scan_results);
    return 0;
  }

  if(strcmp(line,"STATUS")==0){
    char peer[18]={0}; hfp_ag_get_peer_mac(a->hfp,peer);
    snprintf(out,out_sz,"peer=%s number=%s audio=%s auto=%d",
      peer[0]?peer:"(none)",
      a->st.num[0]?a->st.num:"(unset)",
      a->use_opensles?"opensles":"alsa",
      a->st.auto_enabled);
    return 0;
  }

  if(strcmp(line,"SCAN_RESULTS")==0){
    bt_mgmt_scan_results(out,out_sz);
    snprintf(a->st.scan_results,sizeof(a->st.scan_results),"%s", out);
    return 0;
  }

  if(strncmp(line,"AUTO ",5)==0){
    if(strcmp(line+5,"ON")==0){ a->st.auto_enabled=1; snprintf(out,out_sz,"OK"); return 0; }
    if(strcmp(line+5,"OFF")==0){ a->st.auto_enabled=0; snprintf(out,out_sz,"OK"); return 0; }
    snprintf(out,out_sz,"ERR"); return -1;
  }

  if(strncmp(line,"SET_TARGET ",11)==0){
    bdaddr_t b;
    if(bt_parse_mac(line+11, &b)<0){ snprintf(out,out_sz,"ERR"); return -1; }
    a->st.target=b;
    a->st.has_target=1;
    snprintf(out,out_sz,"OK");
    return 0;
  }

  if(strncmp(line,"SET_NUMBER ",11)==0){
    snprintf(a->st.num,sizeof(a->st.num),"%s", line+11);
    hfp_ag_set_number(a->hfp, line+11);
    snprintf(out,out_sz,"OK"); return 0;
  }

  if(strncmp(line,"DIAL ",5)==0){ snprintf(out,out_sz, hfp_ag_dial(a->hfp,line+5)==0?"OK":"ERR"); return 0; }
  if(strcmp(line,"HANGUP")==0){ hfp_ag_hangup(a->hfp); snprintf(out,out_sz,"OK"); return 0; }

  // Manual mgmt override
  if(strcmp(line,"POWER ON")==0){ int rc=bt_mgmt_power(1); a->st.powered=(rc==0); snprintf(out,out_sz, rc==0?"OK":"ERR"); return 0; }
  if(strcmp(line,"POWER OFF")==0){ int rc=bt_mgmt_power(0); a->st.powered=0; snprintf(out,out_sz, rc==0?"OK":"ERR"); return 0; }
  if(strcmp(line,"SCAN ON")==0){ int rc=bt_mgmt_scan(1); a->st.scanning=(rc==0); snprintf(out,out_sz, rc==0?"OK":"ERR"); return 0; }
  if(strcmp(line,"SCAN OFF")==0){ int rc=bt_mgmt_scan(0); a->st.scanning=0; snprintf(out,out_sz, rc==0?"OK":"ERR"); return 0; }
  if(strncmp(line,"PAIR ",5)==0){ snprintf(out,out_sz, bt_mgmt_pair(line+5)==0?"OK":"ERR"); return 0; }
  if(strncmp(line,"CONNECT ",8)==0){ snprintf(out,out_sz, bt_mgmt_connect(line+8)==0?"OK":"ERR"); return 0; }
  if(strncmp(line,"DISCONNECT ",11)==0){ snprintf(out,out_sz, bt_mgmt_disconnect(line+11)==0?"OK":"ERR"); return 0; }

  snprintf(out,out_sz,"ERR");
  return -1;
}

static void usage(){
  LOGI("bt_chatd (C v3.2)");
  LOGI("  --adapter-index N");
  LOGI("  --tcp ip:port | --unix path");
  LOGI("  --audio alsa|opensles");
}

int main(int argc,char** argv){
  const char* tcp="127.0.0.1:3333";
  const char* ux="/dev/socket/bt_chatd";
  const char* audio="alsa";
  int use_tcp=1;
  int adapter_index=0;

  for(int i=1;i<argc;i++){
    if(strcmp(argv[i],"--adapter-index")==0 && i+1<argc) adapter_index=atoi(argv[++i]);
    else if(strcmp(argv[i],"--tcp")==0 && i+1<argc){ tcp=argv[++i]; use_tcp=1; }
    else if(strcmp(argv[i],"--unix")==0 && i+1<argc){ ux=argv[++i]; use_tcp=0; }
    else if(strcmp(argv[i],"--audio")==0 && i+1<argc){ audio=argv[++i]; }
    else if(strcmp(argv[i],"--help")==0 || strcmp(argv[i],"-h")==0){ usage(); return 0; }
  }

  app_t app; memset(&app,0,sizeof(app));
  app.use_opensles = (strcmp(audio,"opensles")==0);

  app.st.adapter_index=adapter_index;
  app.st.mgmt_ready=0;
  app.st.powered=0;
  app.st.scanning=0;
  app.st.auto_enabled=0;
  app.st.has_target=0;
  app.st.connected=0;
  app.st.rfcomm_connected=0;
  app.st.sco_running=0;
  app.st.scan_results[0]=0;
  app.st.num[0]=0;

  bt_mgmt_set_state_ptr(&app.st);
  if(bt_mgmt_init(adapter_index)==0) app.st.mgmt_ready=1;

  app.hfp = hfp_ag_start(app.use_opensles);
  if(!app.hfp){ LOGE("HFP AG start failed"); return 2; }

  app.sm = sm_start(&app.st);

  ctrl_server_t* cs=0;
  if(use_tcp){
    char ip[64]="127.0.0.1"; int port=3333;
    const char* c=strchr(tcp,':');
    if(c){
      size_t n=(size_t)(c-tcp);
      if(n>=sizeof(ip)) n=sizeof(ip)-1;
      memcpy(ip,tcp,n); ip[n]=0;
      port=atoi(c+1);
    } else {
      snprintf(ip,sizeof(ip),"%s",tcp);
    }
    cs=ctrl_server_tcp_start(ip,port,on_cmd,&app);
  }else{
    cs=ctrl_server_unix_start(ux,on_cmd,&app);
  }
  if(!cs){ LOGE("control server start failed"); return 3; }

  LOGI("bt_chatd running");
  while(1) sleep(1);
  return 0;
}
