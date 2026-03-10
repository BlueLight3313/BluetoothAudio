#pragma once
#include "third_party/linux_uapi_bt/bluetooth.h"
typedef struct app_state {
    int adapter_index;
    int mgmt_ready;
    int powered;
    int scanning;
    int auto_enabled;
    int has_target;
    bdaddr_t target;
    int connected;
    bdaddr_t connected_addr;
    int rfcomm_connected;
    int sco_running;
    char scan_results[512];
    char num[64];
} app_state_t;

typedef struct state_machine state_machine_t;
state_machine_t* sm_start(app_state_t* st);
void sm_stop(state_machine_t* sm);
