#include "sco.h"
#include "audio.h"

#include <bluetooth/bluetooth.h>
#include <bluetooth/sco.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>

void* sco_thread(void *arg)
{
    int sock;
    struct sockaddr_sco addr;

    sock = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_SCO);
    if(sock < 0){
        perror("SCO socket");
        return NULL;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sco_family = AF_BLUETOOTH;
    bacpy(&addr.sco_bdaddr, BDADDR_ANY);

    if(bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0){
        perror("SCO bind");
        close(sock);
        return NULL;
    }

    listen(sock, 1);

    printf("[INFO] SCO thread waiting for audio connection...\n");

    while(1)
    {
        int client = accept(sock, NULL, NULL);
        if(client < 0){
            perror("SCO accept");
            continue;
        }

        printf("[INFO] SCO audio connected\n");

        char buf[120];

        while(1)
        {
            int n = read(client, buf, sizeof(buf));

            if(n <= 0)
                break;

            /* headset -> speaker */
            audio_play(buf, n);

            /* microphone -> headset */
            int m = audio_capture(buf, sizeof(buf));

            if(m > 0)
                write(client, buf, m);
        }

        printf("[INFO] SCO audio disconnected\n");

        close(client);
    }

    close(sock);
    return NULL;
}