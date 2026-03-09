#ifndef HCI_H
#define HCI_H

#include <stdint.h>

int hci_open(const char *dev);
int hci_send_cmd(int fd,uint16_t opcode,const void *data,uint8_t len);
int hci_read_event(int fd,uint8_t *buf);

#endif // HCI_H