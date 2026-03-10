#pragma once
typedef struct hfp_ag hfp_ag_t;
hfp_ag_t* hfp_ag_start(int use_opensles);
void hfp_ag_stop(hfp_ag_t* a);
void hfp_ag_set_number(hfp_ag_t* a,const char* s);
int  hfp_ag_get_peer_mac(hfp_ag_t* a,char out[18]);
int  hfp_ag_dial(hfp_ag_t* a,const char* mac_str);
int  hfp_ag_hangup(hfp_ag_t* a);
