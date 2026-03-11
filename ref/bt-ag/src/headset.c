#include "headset.h"
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

headset_t paired_headsets[MAX_HEADSETS];
int paired_count = 0;

headset_t* find_headset(bdaddr_t *bd) {
    for(int i=0;i<paired_count;i++)
        if(!bacmp(&paired_headsets[i].bdaddr, bd))
            return &paired_headsets[i];
    return NULL;
}

void scan_and_pair() {
    inquiry_info *info = NULL;
    int max_rsp = 255, num_rsp;
    int dev_id = hci_get_route(NULL);
    int sock = hci_open_dev(dev_id);
    info = malloc(max_rsp * sizeof(inquiry_info));
    num_rsp = hci_inquiry(dev_id, 8, max_rsp, NULL, &info, IREQ_CACHE_FLUSH);
    for(int i=0;i<num_rsp;i++){
        char addr[19], name[248];
        ba2str(&info[i].bdaddr, addr);
        if(hci_read_remote_name(sock, &info[i].bdaddr, sizeof(name), name, 0)<0)
            strcpy(name,"unknown");
        int exists = 0;
        for(int j=0;j<paired_count;j++)
            if(!bacmp(&paired_headsets[j].bdaddr, &info[i].bdaddr))
                exists = 1;
        if(!exists && paired_count<MAX_HEADSETS){
            bacpy(&paired_headsets[paired_count].bdaddr, &info[i].bdaddr);
            strncpy(paired_headsets[paired_count].name,name,247);
            paired_headsets[paired_count].connected=0;
            paired_headsets[paired_count].sco_active=0;
            paired_headsets[paired_count].sms_supported=1;
            paired_headsets[paired_count].was_connected=0;
            paired_count++;
            printf("[INFO] Paired headset: %s %s\n", addr, name);
        }
    }
    free(info); close(sock);
}