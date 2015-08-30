#ifndef __SERIAL_H__
#define __SERIAL_H__

#define DEFAULT_BAUD    9600

typedef struct tagSerial {
    int fd;
    int baud;
    char *dev;
    
} Serial;

Serial *serialNew();
int serialInit(const char *dev, int baud);
int serialFlush(int fd);
int serialClose(int fd);

#endif
