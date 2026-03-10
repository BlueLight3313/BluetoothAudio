#pragma once
#include <stdint.h>
#include "bluetooth.h"
#define AF_BLUETOOTH 31
#define BTPROTO_RFCOMM 3
#define SOCK_STREAM 1
struct sockaddr_rc { uint16_t rc_family; bdaddr_t rc_bdaddr; uint8_t rc_channel; };
