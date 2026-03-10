#pragma once
#include <stdint.h>
#include "bluetooth.h"
#define AF_BLUETOOTH 31
#define BTPROTO_SCO 2
#define SOCK_SEQPACKET 5
struct sockaddr_sco { uint16_t sco_family; bdaddr_t sco_bdaddr; };
