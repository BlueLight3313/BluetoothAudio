#pragma once
int bt_mgmt_init(int adapter_index);
void bt_mgmt_close(void);
int bt_mgmt_power(int on);
int bt_mgmt_scan(int on);
int bt_mgmt_pair(const char* mac);
int bt_mgmt_connect(const char* mac);
int bt_mgmt_disconnect(const char* mac);
int bt_mgmt_scan_results(char* out, int out_sz);

// v3.2: allow mgmt to update an app_state_t-like struct without coupling headers
void bt_mgmt_set_state_ptr(void* app_state_ptr);
