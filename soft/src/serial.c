#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "serial.h"

int open_port(void) {
    int fd, n;
    char commandBuf[100];
    
    fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY);

    if(fd == -1) {
        return -1;
    }

    while(commandBuf[0] != 'q' ||
          commandBuf[1] != 'u' ||
          commandBuf[2] != 'i' ||
          commandBuf[3] != 't') {
        n = write(fd, "reboot\r\n", 8);
        if(n < 0) {
            return -2;
        }
    }

    close(fd);

    return 0;
}
