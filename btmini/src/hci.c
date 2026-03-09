#include "hci.h"
#include "bt_defs.h"

#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>

int hci_open(const char *dev)
{
    int fd;

    fd = open(dev,O_RDWR|O_NOCTTY);

    if(fd<0)
        perror("HCI open");

    return fd;
}

int hci_send_cmd(int fd,uint16_t opcode,const void *data,uint8_t len)
{
    uint8_t buf[260];

    buf[0]=HCI_CMD_PKT;
    buf[1]=opcode &0xff;
    buf[2]=opcode >>8;
    buf[3]=len;

    if(len>0)
        memcpy(buf+4,data,len);

    return write(fd,buf,len+4);
}

int hci_read_event(int fd,uint8_t *buf)
{
    return read(fd,buf,MAX_PACKET_SIZE);
}