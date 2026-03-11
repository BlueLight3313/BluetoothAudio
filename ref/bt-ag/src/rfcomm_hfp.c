#include "rfcomm_hfp.h"
#include "sms.h"
#include "headset.h"
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int sco_busy = 0;
headset_t *active_headset = NULL;

void simulate_call(int sock){
    write(sock,"+CIEV: 3,1\r\n",11);
    write(sock,"+CIEV: 2,1\r\n",11);
    write(sock,"+CIEV: 3,0\r\n",11);
}

void* rfcomm_thread(void* arg){
    int sock = socket(AF_BLUETOOTH,SOCK_STREAM,BTPROTO_RFCOMM);
    struct sockaddr_rc addr={0};
    addr.rc_family = AF_BLUETOOTH; addr.rc_channel=1;
    bacpy(&addr.rc_bdaddr,BDADDR_ANY);
    bind(sock,(struct sockaddr*)&addr,sizeof(addr));
    listen(sock,1);
    printf("[INFO] RFCOMM waiting for headset connections...\n");
    while(1){
        struct sockaddr_rc client_addr;
        socklen_t opt = sizeof(client_addr);
        int client = accept(sock,(struct sockaddr*)&client_addr,&opt);
        char str[19]; ba2str(&client_addr.rc_bdaddr,str);
        headset_t *hs = find_headset(&client_addr.rc_bdaddr);
        if(!hs){ printf("[INFO] Unknown headset %s\n",str); close(client); continue;}
        hs->connected = 1; hs->rfcomm_sock = client; active_headset = hs;
        handle_reconnect(hs);
        printf("[INFO] RFCOMM connected: %s\n",hs->name);
        char buf[1024];
        while(1){
            int n = read(client,buf,sizeof(buf)-1);
            if(n<=0) break; buf[n]=0;
            if(strstr(buf,"AT+CMER")){ write(client,"OK\r\n",4);
                if(!sco_busy){ simulate_call(client); sco_busy=1; hs->sco_active=1;}
                else write(client,"ERROR: SCO busy\r\n",17);}
            else if(strstr(buf,"AT+BRSF")){ hs->sms_supported=1; write(client,"+BRSF: 63\r\nOK\r\n",16);}
            else if(strstr(buf,"AT+CMGF")) write(client,"OK\r\n",4);
            else if(strstr(buf,"AT+CNMI")) write(client,"OK\r\n",4);
            else if(strstr(buf,"AT+CMGS=")){ write(client,"> ",2);
                n=read(client,buf,sizeof(buf)-1); buf[n]=0;
                printf("[INFO] SMS from headset: %s\n",buf);
                write(client,"+CMGS: 1\r\n",10); write(client,"OK\r\n",4);}
            else if(strstr(buf,"AT+CMGR")) write(client,"+CMGR: 1,\"+1234567890\",\"Device\",\"Hello from AG\"\r\nOK\r\n",55);
            else write(client,"OK\r\n",4);
        }
        if(hs->sco_active) sco_busy=0;
        hs->connected=0; hs->sco_active=0; active_headset=NULL;
        close(client); printf("[INFO] RFCOMM disconnected: %s\n",hs->name);
    }
    return NULL;
}