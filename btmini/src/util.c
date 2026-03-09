#include <stdio.h>
#include "util.h"

void hex_dump(const uint8_t *data,int len)
{
    int i;

    for(i=0;i<len;i++)
    {
        printf("%02X ",data[i]);

        if((i+1)%16==0)
            printf("\n");
    }

    printf("\n");
}