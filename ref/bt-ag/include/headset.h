#ifndef HEADSET_H
#define HEADSET_H

#include <bluetooth/bluetooth.h>

typedef struct {
    bdaddr_t bdaddr;
    char name[248];
    int connected;
    int sco_active;
    int sms_supported;
    int rfcomm_sock;
    int was_connected;
} headset_t;

#define MAX_HEADSETS 10
extern headset_t paired_headsets[MAX_HEADSETS];
extern int paired_count;

headset_t* find_headset(bdaddr_t *bd);
void scan_and_pair();

#endif