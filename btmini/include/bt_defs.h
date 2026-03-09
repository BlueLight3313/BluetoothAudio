#ifndef BT_DEFS_H
#define BT_DEFS_H

#include <stdint.h>

#define HCI_CMD_PKT 0x01
#define HCI_ACL_PKT 0x02
#define HCI_EVT_PKT 0x04

#define MAX_PACKET_SIZE 1024

typedef struct
{
    uint8_t addr[6];
    char name[64];
} bt_device_t;

#endif // BT_DEFS_H