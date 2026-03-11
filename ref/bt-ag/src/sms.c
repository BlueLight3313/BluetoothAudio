#include "sms.h"
#include <unistd.h>
#include <stdio.h>

device_status_t device_status = DEVICE_IDLE;

void send_sms_to_headset(headset_t *hs, const char *sender, const char *msg){
    if(!hs || !hs->connected || !hs->sms_supported) return;
    char buf[256];
    snprintf(buf,sizeof(buf),"+CMTI: \"SM\",1\r\n");
    write(hs->rfcomm_sock,buf,strlen(buf));
    snprintf(buf,sizeof(buf),"+CMGR: 1,\"%s\",\"Device\",\"%s\"\r\n",sender,msg);
    write(hs->rfcomm_sock,buf,strlen(buf));
}

void broadcast_sms(const char *sender, const char *msg){
    extern int paired_count;
    extern headset_t paired_headsets[];
    for(int i=0;i<paired_count;i++)
        if(paired_headsets[i].connected)
            send_sms_to_headset(&paired_headsets[i],sender,msg);
}

void handle_reconnect(headset_t *hs) {
    if(!hs) return;
    if(!hs->was_connected && hs->connected){
        printf("[INFO] Headset reconnected: %s\n", hs->name);
        if(hs->sms_supported){
            const char *msg;
            switch(device_status){
                case DEVICE_IDLE: msg="Device is idle."; break;
                case DEVICE_BUSY: msg="Device is busy."; break;
                case DEVICE_ALERT: msg="Device alert!"; break;
                default: msg="Device status unknown."; break;
            }
            send_sms_to_headset(hs,"+Device",msg);
        }
    }
    hs->was_connected = hs->connected;
}

void* reconnect_monitor(void *arg){
    extern int paired_count;
    extern headset_t paired_headsets[];
    while(1){
        for(int i=0;i<paired_count;i++)
            handle_reconnect(&paired_headsets[i]);
        sleep(2);
    }
    return NULL;
}