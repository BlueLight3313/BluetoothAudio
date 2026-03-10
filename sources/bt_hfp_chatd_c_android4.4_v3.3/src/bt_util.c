#include "bt_util.h"
#include <stdio.h>
#include <string.h>
int bt_parse_mac(const char* s, bdaddr_t* out){
    if(!s||!out) return -1;
    int v[6];
    if(sscanf(s,"%x:%x:%x:%x:%x:%x",&v[0],&v[1],&v[2],&v[3],&v[4],&v[5])!=6) return -1;
    for(int i=0;i<6;i++) out->b[5-i]=(unsigned char)(v[i]&0xFF);
    return 0;
}
void bt_format_mac(const bdaddr_t* a, char out[18]){
    snprintf(out,18,"%02X:%02X:%02X:%02X:%02X:%02X", a->b[5],a->b[4],a->b[3],a->b[2],a->b[1],a->b[0]);
}
void bt_bdaddr_any(bdaddr_t* out){ memset(out,0,sizeof(*out)); }
