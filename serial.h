#ifndef __SERIAL_H__
#define __SERIAL_H__

#include "config.h"

#define MAX_BUFFER_SIZE     256

typedef struct tagSerial {
    int fd;
    int baud;
    char *dev;
    
    int bufflen;
    char buff[MAX_BUFFER_SIZE];
    
} Serial;

Serial *serialNew();
int serialInit(const char *dev, int baud);
int serialRead(Serial *pstSerial);
int serialFlush(int fd);
int serialClose(int fd);

#endif
