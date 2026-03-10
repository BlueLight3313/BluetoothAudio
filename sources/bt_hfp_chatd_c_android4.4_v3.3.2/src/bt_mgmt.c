#include "bt_mgmt.h"
#include "log.h"
#include "bt_util.h"
#include "third_party/linux_uapi_bt/hci.h"
#include "third_party/linux_uapi_bt/bluetooth.h"
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <poll.h>
#include <stdlib.h>
#include <pthread.h>

#pragma pack(push,1)
struct mgmt_hdr { unsigned short opcode; unsigned short index; unsigned short len; };
#pragma pack(pop)

#define MGMT_OP_SET_POWERED        0x0005
#define MGMT_OP_SET_CONNECTABLE    0x0006
#define MGMT_OP_SET_DISCOVERABLE   0x0007
#define MGMT_OP_START_DISCOVERY    0x0023
#define MGMT_OP_STOP_DISCOVERY     0x0024
#define MGMT_OP_PAIR_DEVICE        0x0019
#define MGMT_OP_CONNECT            0x001B
#define MGMT_OP_DISCONNECT         0x001C
#define MGMT_OP_PIN_CODE_REPLY     0x000E
#define MGMT_OP_USER_CONFIRM_REPLY 0x0010

#define MGMT_EV_CMD_COMPLETE       0x0001
#define MGMT_EV_CMD_STATUS         0x0002
#define MGMT_EV_DEVICE_FOUND       0x0012
#define MGMT_EV_DEVICE_CONNECTED   0x0014
#define MGMT_EV_DEVICE_DISCONNECTED 0x0015
#define MGMT_EV_PIN_CODE_REQUEST   0x000D
#define MGMT_EV_USER_CONFIRM_REQUEST 0x000F

#define MGMT_ADDR_BREDR 0x00
#define MGMT_IO_CAP_NO_INPUT_NO_OUTPUT 0x03

static int g_fd=-1;
static int g_index=0;
static void* g_state_ptr=NULL;

#define SCAN_MAX 20
struct scan_entry { char mac[18]; signed char rssi; };
static struct scan_entry g_scan[SCAN_MAX];
static int g_scan_count=0;

static void scan_add(const char* mac, signed char rssi){
    for(int i=0;i<g_scan_count;i++){
        if(strcmp(g_scan[i].mac,mac)==0){ g_scan[i].rssi=rssi; return; }
    }
    if(g_scan_count<SCAN_MAX){
        snprintf(g_scan[g_scan_count].mac,sizeof(g_scan[g_scan_count].mac),"%s",mac);
        g_scan[g_scan_count].rssi=rssi;
        g_scan_count++;
    }else{
        memmove(&g_scan[0],&g_scan[1],sizeof(g_scan[0])*(SCAN_MAX-1));
        snprintf(g_scan[SCAN_MAX-1].mac,sizeof(g_scan[SCAN_MAX-1].mac),"%s",mac);
        g_scan[SCAN_MAX-1].rssi=rssi;
    }
}

static int send_cmd(unsigned short opcode, const void* payload, unsigned short plen){
    if(g_fd<0) return -1;
    unsigned char buf[1024];
    struct mgmt_hdr h; h.opcode=opcode; h.index=(unsigned short)g_index; h.len=plen;
    if(sizeof(h)+plen>sizeof(buf)) return -1;
    memcpy(buf,&h,sizeof(h));
    if(plen && payload) memcpy(buf+sizeof(h),payload,plen);
    ssize_t n=send(g_fd,buf,sizeof(h)+plen,0);
    return (n==(ssize_t)(sizeof(h)+plen))?0:-1;
}

static int recv_one(unsigned char* buf, int cap, int timeout_ms){
    struct pollfd pfd; pfd.fd=g_fd; pfd.events=POLLIN;
    int pr=poll(&pfd,1,timeout_ms);
    if(pr<=0) return pr;
    ssize_t n=recv(g_fd,buf,cap,0);
    if(n<=0) return -1;
    return (int)n;
}

// app_state_t layout (must match initial fields we touch)
struct minimal_state {
    int adapter_index;
    int mgmt_ready;
    int powered;
    int scanning;
    int auto_enabled;
    int has_target;
    bdaddr_t target;
    int connected;
    bdaddr_t connected_addr;
    int rfcomm_connected;
    int sco_running;
    char scan_results[512];
    char num[64];
};

static void state_set_connected(int connected, const bdaddr_t* addr){
    if(!g_state_ptr) return;
    struct minimal_state* st=(struct minimal_state*)g_state_ptr;
    st->connected = connected;
    if(addr) st->connected_addr = *addr;
}

static pthread_t g_th;
static volatile int g_run=0;

static void* listener_main(void* p){
    (void)p;
    unsigned char buf[1024];
    while(g_run){
        int n=recv_one(buf,(int)sizeof(buf),200);
        if(n<=0) continue;
        if(n<(int)sizeof(struct mgmt_hdr)) continue;
        struct mgmt_hdr* h=(struct mgmt_hdr*)buf;
        if(h->index!=(unsigned short)g_index) continue;

        if(h->opcode==MGMT_EV_DEVICE_FOUND){
            if(h->len>=6+1+1){
                bdaddr_t a; memcpy(&a,buf+sizeof(*h),6);
                char mac[18]; bt_format_mac(&a,mac);
                signed char rssi=*(signed char*)(buf+sizeof(*h)+6+1);
                scan_add(mac,rssi);
            }
            continue;
        }
        if(h->opcode==MGMT_EV_DEVICE_CONNECTED){
            if(h->len>=6+1){
                bdaddr_t a; memcpy(&a,buf+sizeof(*h),6);
                state_set_connected(1,&a);
            }
            continue;
        }
        if(h->opcode==MGMT_EV_DEVICE_DISCONNECTED){
            state_set_connected(0,NULL);
            continue;
        }

        if(h->opcode==MGMT_EV_PIN_CODE_REQUEST){
            if(h->len>=8){
                bdaddr_t a; memcpy(&a,buf+sizeof(*h),6);
                unsigned char addr_type=*(unsigned char*)(buf+sizeof(*h)+6);
                struct __attribute__((packed)) { bdaddr_t addr; unsigned char addr_type; unsigned char pin_len; char pin[16]; } rep;
                memset(&rep,0,sizeof(rep));
                rep.addr=a; rep.addr_type=addr_type; rep.pin_len=4; memcpy(rep.pin,"0000",4);
                send_cmd(MGMT_OP_PIN_CODE_REPLY,&rep,(unsigned short)(6+1+1+rep.pin_len));
            }
            continue;
        }
        if(h->opcode==MGMT_EV_USER_CONFIRM_REQUEST){
            if(h->len>=11){
                bdaddr_t a; memcpy(&a,buf+sizeof(*h),6);
                unsigned char addr_type=*(unsigned char*)(buf+sizeof(*h)+6);
                struct __attribute__((packed)) { bdaddr_t addr; unsigned char addr_type; unsigned char confirm; } rep;
                rep.addr=a; rep.addr_type=addr_type; rep.confirm=1;
                send_cmd(MGMT_OP_USER_CONFIRM_REPLY,&rep,(unsigned short)(6+1+1));
            }
            continue;
        }
    }
    return NULL;
}

static int wait_cmd(unsigned short opcode, int timeout_ms){
    unsigned char buf[1024];
    int remain=timeout_ms;
    while(remain>0){
        int n=recv_one(buf,(int)sizeof(buf),200);
        remain-=200;
        if(n<=0) continue;
        if(n<(int)sizeof(struct mgmt_hdr)) continue;
        struct mgmt_hdr* h=(struct mgmt_hdr*)buf;
        if(h->index!=(unsigned short)g_index) continue;
        if(h->opcode==MGMT_EV_CMD_STATUS || h->opcode==MGMT_EV_CMD_COMPLETE){
            if(h->len<3) continue;
            unsigned short op=0; memcpy(&op,buf+sizeof(*h),2);
            unsigned char status=*(unsigned char*)(buf+sizeof(*h)+2);
            if(op!=opcode) continue;
            return status==0?0:-1;
        }
    }
    return -1;
}

void bt_mgmt_set_state_ptr(void* app_state_ptr){ g_state_ptr=app_state_ptr; }

int bt_mgmt_init(int adapter_index){
    if(g_fd>=0) return 0;
    g_index=adapter_index;
    g_fd=socket(AF_BLUETOOTH,SOCK_RAW,BTPROTO_HCI);
    if(g_fd<0){ LOGE("mgmt socket failed: %s", strerror(errno)); return -1; }
    struct sockaddr_hci a; memset(&a,0,sizeof(a));
    a.hci_family=AF_BLUETOOTH; a.hci_dev=HCI_DEV_NONE; a.hci_channel=HCI_CHANNEL_CONTROL;
    if(bind(g_fd,(struct sockaddr*)&a,sizeof(a))<0){
        LOGE("mgmt bind(control) failed: %s", strerror(errno));
        close(g_fd); g_fd=-1; return -1;
    }
    g_scan_count=0;
    g_run=1;
    pthread_create(&g_th,NULL,listener_main,NULL);
    LOGI("mgmt init ok index=%d", g_index);
    return 0;
}

void bt_mgmt_close(void){
    if(g_fd>=0){
        g_run=0;
        pthread_join(g_th,NULL);
        close(g_fd);
    }
    g_fd=-1;
}

int bt_mgmt_power(int on){
    if(bt_mgmt_init(g_index)<0) return -1;
    unsigned char v=on?1:0;
    if(send_cmd(MGMT_OP_SET_POWERED,&v,1)<0) return -1;
    return wait_cmd(MGMT_OP_SET_POWERED,3000);
}

int bt_mgmt_scan(int on){
    if(bt_mgmt_init(g_index)<0) return -1;
    if(on){
        unsigned char v=1;
        send_cmd(MGMT_OP_SET_CONNECTABLE,&v,1); (void)wait_cmd(MGMT_OP_SET_CONNECTABLE,2000);
        send_cmd(MGMT_OP_SET_DISCOVERABLE,&v,1); (void)wait_cmd(MGMT_OP_SET_DISCOVERABLE,2000);
        unsigned char type=0x01;
        g_scan_count=0;
        if(send_cmd(MGMT_OP_START_DISCOVERY,&type,1)<0) return -1;
        return wait_cmd(MGMT_OP_START_DISCOVERY,3000);
    }else{
        unsigned char type=0x01;
        if(send_cmd(MGMT_OP_STOP_DISCOVERY,&type,1)<0) return -1;
        return wait_cmd(MGMT_OP_STOP_DISCOVERY,3000);
    }
}

int bt_mgmt_pair(const char* mac){
    if(!mac) return -1;
    if(bt_mgmt_init(g_index)<0) return -1;
    bdaddr_t addr; if(bt_parse_mac(mac,&addr)<0) return -1;
    struct __attribute__((packed)) { bdaddr_t addr; unsigned char addr_type; unsigned char io_cap; } p;
    p.addr=addr; p.addr_type=MGMT_ADDR_BREDR; p.io_cap=MGMT_IO_CAP_NO_INPUT_NO_OUTPUT;
    if(send_cmd(MGMT_OP_PAIR_DEVICE,&p,(unsigned short)sizeof(p))<0) return -1;
    return wait_cmd(MGMT_OP_PAIR_DEVICE,15000);
}

int bt_mgmt_connect(const char* mac){
    if(!mac) return -1;
    if(bt_mgmt_init(g_index)<0) return -1;
    bdaddr_t addr; if(bt_parse_mac(mac,&addr)<0) return -1;
    struct __attribute__((packed)) { bdaddr_t addr; unsigned char addr_type; } p;
    p.addr=addr; p.addr_type=MGMT_ADDR_BREDR;
    if(send_cmd(MGMT_OP_CONNECT,&p,(unsigned short)sizeof(p))<0) return -1;
    return wait_cmd(MGMT_OP_CONNECT,8000);
}

int bt_mgmt_disconnect(const char* mac){
    if(!mac) return -1;
    if(bt_mgmt_init(g_index)<0) return -1;
    bdaddr_t addr; if(bt_parse_mac(mac,&addr)<0) return -1;
    struct __attribute__((packed)) { bdaddr_t addr; unsigned char addr_type; } p;
    p.addr=addr; p.addr_type=MGMT_ADDR_BREDR;
    if(send_cmd(MGMT_OP_DISCONNECT,&p,(unsigned short)sizeof(p))<0) return -1;
    return wait_cmd(MGMT_OP_DISCONNECT,8000);
}

int bt_mgmt_scan_results(char* out, int out_sz){
    if(!out||out_sz<=0) return -1;
    int n=0;
    if(g_scan_count==0){ snprintf(out,out_sz,"(empty)"); return 0; }
    for(int i=0;i<g_scan_count;i++){
        n += snprintf(out+n,out_sz-n,"%s(rssi=%d)%s", g_scan[i].mac,(int)g_scan[i].rssi,(i==g_scan_count-1)?"":" ");
        if(n>=out_sz) break;
    }
    return 0;
}
