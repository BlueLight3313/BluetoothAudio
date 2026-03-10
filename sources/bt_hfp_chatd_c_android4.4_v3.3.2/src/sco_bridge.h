#pragma once
#include "third_party/linux_uapi_bt/bluetooth.h"
typedef struct sco_bridge sco_bridge_t;
sco_bridge_t* sco_bridge_start(const bdaddr_t* remote, int use_opensles);
void sco_bridge_stop(sco_bridge_t* b);
