#ifndef ACL_H
#define ACL_H

#include <stdint.h>

int acl_send(int fd, uint16_t handle, uint8_t *data, int len);

#endif // ACL_H
