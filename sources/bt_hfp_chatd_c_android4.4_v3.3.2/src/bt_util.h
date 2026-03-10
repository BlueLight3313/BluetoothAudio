#pragma once
#include "third_party/linux_uapi_bt/bluetooth.h"
int  bt_parse_mac(const char* s, bdaddr_t* out);
void bt_format_mac(const bdaddr_t* a, char out[18]);
void bt_bdaddr_any(bdaddr_t* out);
