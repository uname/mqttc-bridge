#include "serial.h"
#include "log.h"

#include <assert.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>

Serial *serialNew()
{
    Serial *serial = NULL;
    
    serial = (Serial *)malloc(sizeof(Serial));
    assert(serial != NULL);
    
    serial->fd = 0;
    serial->dev = NULL;
    serial->baud = DEFAULT_BAUD;
    return serial;
}

int serialInit(const char *dev, int baud)
{
    struct termios toptions;
    int fd;
    
    if(dev == NULL) {
        LOG_E("dev is null\n");
        return -1;
    }
    
    LOG_I("init serial device %s ...\n", dev);
    fd = open(dev, O_RDWR | O_NONBLOCK );
    
    if (fd == -1)  {
        LOG_E("serial_init: Unable to open port\n");
        return -1;
    }
    
    if (tcgetattr(fd, &toptions) < 0) {
        LOG_E("serial_init: Couldn't get term attributes\n");
        return -1;
    }

    speed_t brate = baud;

    switch(baud) {
    case 4800:   brate = B4800;   break;
    case 9600:   brate = B9600;   break;
#ifdef B14400
    case 14400:  brate = B14400;  break;
#endif
    case 19200:  brate = B19200;  break;
#ifdef B28800
    case 28800:  brate = B28800;  break;
#endif
    case 38400:  brate = B38400;  break;
    case 57600:  brate = B57600;  break;
    case 115200: brate = B115200; break;
    
    default:
        LOG_E("baud not supported\n");
        return -1;
    }
    
    cfsetispeed(&toptions, brate);
    cfsetospeed(&toptions, brate);

    // 8N1
    toptions.c_cflag &= ~PARENB;
    toptions.c_cflag &= ~CSTOPB;
    toptions.c_cflag &= ~CSIZE;
    toptions.c_cflag |= CS8;

    toptions.c_cflag &= ~CRTSCTS; // no flow control

    toptions.c_cflag |= CREAD | CLOCAL;  // turn on READ & ignore ctrl lines
    toptions.c_iflag &= ~(IXON | IXOFF | IXANY); // turn off s/w flow ctrl

    toptions.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // make raw
    toptions.c_oflag &= ~OPOST; // make raw

    toptions.c_cc[VMIN]  = 0;
    toptions.c_cc[VTIME] = 0;
    
    tcsetattr(fd, TCSANOW, &toptions);
    if( tcsetattr(fd, TCSAFLUSH, &toptions) < 0) {
        LOG_E("could not set term attributes\n");
        return -1;
    }

    return fd;
}

int serialRead(Serial *pstSerial)
{
    if(pstSerial == NULL) {
        LOG_E("pstSerial is null\n");
        return -1;
    }
    pstSerial->bufflen = read(pstSerial->fd, pstSerial->buff, MAX_BUFFER_SIZE);
    return pstSerial->bufflen;
}

int serialFlush(int fd)
{
    int ret;
    usleep(10000); 
    ret = tcflush(fd, TCIOFLUSH);
    usleep(1000000); 
    return ret;
}

int serialClose(int fd)
{
    return close(fd);
}
