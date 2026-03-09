#include "l2cap.h"


int l2cap_connect(uint16_t handle,uint16_t psm)
{
    uint8_t pkt[12];

    pkt[0]=0x02;
    pkt[1]=0x00;

    pkt[2]=0x08;
    pkt[3]=0x00;

    pkt[4]=0x01;
    pkt[5]=0x01;

    pkt[6]=psm &0xff;
    pkt[7]=psm >>8;

    pkt[8]=0x40;
    pkt[9]=0x00;

    return acl_send(bt_fd,handle,pkt,10);
}