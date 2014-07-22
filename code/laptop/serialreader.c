#include "serialreader.h"

#define SYNC_INT (('S'<<8) | 'Y')
#define ARDUINO_TTY "/dev/arduinomicro"

void* serialreader (serial_vars *sv) {
    struct termios options;

    int fd = open(ARDUINO_TTY, O_RDONLY);
    if (fd == -1) {
        perror(ARDUINO_TTY);
        return 0;

    }

    fcntl(fd, F_SETFL, 0);                  /* Configure port reading */
                                     /* Get the current options for the port */
    tcgetattr(fd, &options);
    cfsetispeed(&options, B115200);                 /* Set the baud rates to 9600 */
    cfsetospeed(&options, B115200);

                                   /* Enable the receiver and set local mode */
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~PARENB; /* Mask the character size to 8 bits, no parity */
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |=  CS8;                              /* Select 8 data bits */
    options.c_cflag &= ~CRTSCTS;               /* Disable hardware flow control */ 

    //Block for data
    options.c_cc[VMIN]  = 1;
    options.c_cc[VTIME] = 0;

                                 /* Enable data to be processed as raw input */
    options.c_lflag &= ~(ICANON | ECHO | ISIG);
       
                                        /* Set the new options for the port */
    tcsetattr(fd, TCSANOW, &options);


    for (;;) {
        uint8_t in;
        uint16_t sync;
        read(fd, &in, 1) > 0;
        sync = (sync<<8) |  in;
        if (sync==SYNC_INT) {
            uint8_t buf[5];
            read(fd, buf, sizeof(buf));
            if (buf[0] ^ buf[1] ^ buf[2] ^ buf[3] == buf[4]) {
                sv->lux = (buf[1] << 8) | buf[0];
                sv->range = (buf[3] << 8) | buf[2];
                printf("lux: %4d   range: %5d\n", sv->lux, sv->range);
            } else {
                printf("error transmission\n");
            }
        }
    }
}