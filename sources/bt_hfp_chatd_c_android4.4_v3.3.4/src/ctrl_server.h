#pragma once
typedef struct ctrl_server ctrl_server_t;
typedef int (*ctrl_handler_fn)(const char* line,char* out,int out_sz,void* user);
ctrl_server_t* ctrl_server_tcp_start(const char* bind_ip,int port,ctrl_handler_fn fn,void* user);
ctrl_server_t* ctrl_server_unix_start(const char* path,ctrl_handler_fn fn,void* user);
void ctrl_server_stop(ctrl_server_t* s);
