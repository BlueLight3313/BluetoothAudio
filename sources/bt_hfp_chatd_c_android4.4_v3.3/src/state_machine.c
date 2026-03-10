#include "state_machine.h"
#include "bt_mgmt.h"
#include "bt_util.h"
#include "log.h"
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

struct state_machine { app_state_t* st; pthread_t th; volatile int running; };

static void* th_main(void* p){
    struct state_machine* sm=(struct state_machine*)p;
    while(sm->running){
        app_state_t* st=sm->st;

        if(st->auto_enabled && st->mgmt_ready){
            if(!st->powered){
                if(bt_mgmt_power(1)==0){
                    st->powered=1;
                    LOGI("AUTO: powered on");
                }
            }

            if(st->has_target && st->powered){
                char mac[18]; bt_format_mac(&st->target, mac);

                if(!st->connected){
                    // short scan burst to refresh
                    (void)bt_mgmt_scan(1);
                    usleep(300*1000);
                    (void)bt_mgmt_scan(0);
                    bt_mgmt_scan_results(st->scan_results, (int)sizeof(st->scan_results));

                    // best-effort pair, then connect
                    (void)bt_mgmt_pair(mac);
                    if(bt_mgmt_connect(mac)==0){
                        st->connected=1;
                        st->connected_addr=st->target;
                        LOGI("AUTO: connected %s", mac);
                    }
                }
            }
        }
        sleep(1);
    }
    return NULL;
}

state_machine_t* sm_start(app_state_t* st){
    if(!st) return NULL;
    struct state_machine* sm=(struct state_machine*)calloc(1,sizeof(*sm));
    sm->st=st; sm->running=1;
    pthread_create(&sm->th,NULL,th_main,sm);
    return (state_machine_t*)sm;
}
void sm_stop(state_machine_t* sm0){
    struct state_machine* sm=(struct state_machine*)sm0;
    if(!sm) return;
    sm->running=0;
    pthread_join(sm->th,NULL);
    free(sm);
}
