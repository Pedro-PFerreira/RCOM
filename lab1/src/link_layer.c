// Link layer protocol implementation

#include "link_layer.h"
#include "state_machine.h"
#include "macros.h"
#include "alarm.c"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>

int state = 0;
char* filename;
int nTries = 0;

unsigned char rcv_buf[MAX_SIZE + 1] = {0};
unsigned char send_buf[MAX_SIZE + 1] = {0};

int bytes = 0;

int fd;

volatile int STOP = FALSE;

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters)
{   

    struct termios oldtio;
    struct termios newtio;

    // Save current port settings
    if (tcgetattr(fd, &oldtio) == -1)
    {
        perror("tcgetattr");
        exit(-1);
    }

    // Clear struct for new port settings
    memset(&newtio, 0, sizeof(newtio));

    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    // Set input mode (non-canonical, no echo,...)
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0; // Inter-character timer unused
    newtio.c_cc[VMIN] = 5;  // Blocking read until 5 chars received

    // VTIME e VMIN should be changed in order to protect with a
    // timeout the reception of the following character(s)

    // Now clean the line and activate the settings for the port
    // tcflush() discards data written to the object referred to
    // by fd but not transmitted, or data received but not read,
    // depending on the value of queue_selector:
    //   TCIFLUSH - flushes data received but not read.
    tcflush(fd, TCIOFLUSH);

    // Set new port settings
    if (tcsetattr(fd, TCSANOW, &newtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    printf("New termios structure set\n");

    int state = START;


    write(fd, rcv_buf, bytes);
    printf("Sent: %s:%d\n", send_buf, bytes);
    // The while() cycle should be changed in order to respect the specifications
    // of the protocol indicated in the Lab guide

    // Restore the old port settings
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    llclose(fd);

    return 0;
    
}

unsigned char stuff(unsigned char * block){

    unsigned char res = 0x0000;
    if (*block == FLAG){
        *block = *block ^ STUFFER;
        res |= ESC;
        res |= (*block & 0x00FF);
    }
    else if (block == ESC){
        res |= (*block ^ STUFFER);
    }

    return res;

}

unsigned char destuff(unsigned char * block){

    unsigned char res= 0x0000;
    res |= (*block ^STUFFER);
    return res;
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(unsigned char *buf, int bufSize)
{       
    unsigned char buf[MAX_SIZE + 1] = {0};
    unsigned char rcv_buf[MAX_SIZE + 1] = {0};
    unsigned char send_buf[MAX_SIZE + 1] = {0};
    rcv_buf[0] = FLAG_RCV;
    rcv_buf[1] = A_RCV;
    rcv_buf[2] = C_RCV;
    rcv_buf[3] = A_RCV ^ C_RCV;
    rcv_buf[4] = FLAG_RCV;

    for (int i = 0; i < bufSize; i++){
        buf[i] = stuff(buf[i]);
    }
    

    return 0;
}



////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet)
{

    int packet_sz = *packet / sizeof(unsigned char);

    *packet = destuff(packet);
    
    while(TRUE){
        if (packet_sz > MAX_SIZE) {
            printf("Can't read packet!\n");
            return 1;
        }
        unsigned char buf[packet_sz - 1];

        for (int i = 0; i < packet_sz; i++){
            packet[i]= buf[i];
        }

        break;       
    }

    return packet_sz;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose(int showStatistics)
{
    set_state(state, START);

    return 1;
}
