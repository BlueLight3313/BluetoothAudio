#pragma once
#include <stdint.h>
#define BTPROTO_HCI 1
#define SOCK_RAW 3
#define HCI_CHANNEL_CONTROL 3
#define HCI_DEV_NONE 0xffff
struct sockaddr_hci { uint16_t hci_family; uint16_t hci_dev; uint16_t hci_channel; };
