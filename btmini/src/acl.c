#include "acl.h"

int acl_send(int fd,uint16_t handle,uint8_t *data,int len)
{
    uint8_t buf[1024];

    buf[0]=0x02;
    buf[1]=handle &0xff;
    buf[2]=handle>>8;

    buf[3]=len &0xff;
    buf[4]=len>>8;

    memcpy(buf+5,data,len);

    return write(fd,buf,len+5);
}