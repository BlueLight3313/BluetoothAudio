#include "headset.h"
#include "audio.h"
#include "rfcomm_hfp.h"
#include "sms.h"
#include "sco.h"
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

int main(){
    printf("[INFO] Multi-pairing HFP Audio Gateway starting\n");
    audio_init();
    scan_and_pair();

    pthread_t rfcomm,sco,monitor;
    pthread_create(&rfcomm,NULL,rfcomm_thread,NULL);
    pthread_create(&sco,NULL,sco_thread,NULL);
    pthread_create(&monitor,NULL,reconnect_monitor,NULL);

    // Example periodic broadcast SMS
    while(1){
        sleep(30);
        broadcast_sms("+1234567890","Hello headset! Device periodic SMS.");
    }

    pthread_join(rfcomm,NULL);
    pthread_join(sco,NULL);
    pthread_join(monitor,NULL);
    return 0;
}