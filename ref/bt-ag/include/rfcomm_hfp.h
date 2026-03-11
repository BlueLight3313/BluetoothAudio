#ifndef RFCOMM_HFP_H
#define RFCOMM_HFP_H

#include "headset.h"

extern int sco_busy;
extern headset_t *active_headset;

void* rfcomm_thread(void* arg);
void simulate_call(int sock);

#endif