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

    if (connectionParameters.role == LlRx){
        //TODO
    }

    else if (connectionParameters.role == LlTx){
        llwrite(send_buf, MAX_SIZE);
    }
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
    unsigned char buf1[MAX_SIZE];

    int BCC_2 = buf[0];  

    for (int i = 1; i < bufSize; i++){
        BCC_2 = BCC_2 ^ buf[i];
    }

    buf1[0] = FLAG_RCV;
    buf1[1] = A_RCV;
    buf1[2] = C_RCV;
    buf1[3] = A_RCV ^ C_RCV;


    buf1[bufSize + 3] = BCC_2; 
    buf1[bufSize + 4] = FLAG_RCV;

    size_t buf1_sz = sizeof(buf1) / sizeof(buf1[0]);

    for (size_t i = 0; i < buf1_sz; i++){
        rcv_buf[i] = buf[i];
    }
    
    size_t rcv_sz = sizeof(rcv_buf) / sizeof(rcv_buf[0]);






    for (int i = 4; i < rcv_sz; i++){
        send_buf[i] = stuff(buf1[i]);
    }

    write(fd, send_buf, bytes);

    return 0;
}



////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet)
{
    destuff(packet);

    return 0;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose(int showStatistics)
{
    set_state(state, START);

    return 1;
}
