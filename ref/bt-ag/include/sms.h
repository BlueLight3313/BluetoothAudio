#ifndef SMS_H
#define SMS_H

#include "headset.h"

typedef enum { DEVICE_IDLE, DEVICE_BUSY, DEVICE_ALERT } device_status_t;
extern device_status_t device_status;

void send_sms_to_headset(headset_t *hs, const char *sender, const char *msg);
void broadcast_sms(const char *sender, const char *msg);
void handle_reconnect(headset_t *hs);
void* reconnect_monitor(void *arg);

#endif