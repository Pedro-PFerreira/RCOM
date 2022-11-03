// Link layer protocol implementation

#include "../include/link_layer.h"
#include "../include/state_machine.h"
#include "../include/macros.h"
#include "../include/alarm.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>

int state = START;
char* filename;
int nTries = 0;

unsigned char rcv_buf[MAX_SIZE + 7];
unsigned char send_buf[MAX_SIZE+ 7];

int bytes = 0;

int fd = 0;

LinkLayerRole role;
int numTries;
int timeout;

volatile int STOP = FALSE;

struct termios oldtio;
struct termios newtio;

int total_timeouts = 0;
int total_retransmits = 0;
int total_rej = 0;
int total_received_frames = 0;
int total_frames_sent = 0;

int S = 0;

void llopen_t(){
    unsigned char set_message[5];
    set_message[0] = FLAG_RCV;
    set_message[1] = A_T;
    set_message[2] = SET;
    set_message[3] = A_T ^ C_T;
    set_message[4] = FLAG_RCV;

    printf("Sent: %x %x %x %x %x\n", set_message[0], set_message[1],set_message[2],set_message[3],set_message[4]);
    int bytes = write(fd, set_message, 5);

    if (bytes == -1){
        return;
    }

    unsigned char ua_message[5];
    bytes = read(fd, ua_message, 5);
    printf("Received: %x %x %x %x %x\n", ua_message[0], ua_message[1],ua_message[2],ua_message[3],ua_message[4]);

}

void llopen_r()
{
    unsigned char set_message[5];
    read(fd, set_message, 5);

    printf("Received: %x %x %x %x %x\n", set_message[0], set_message[1],set_message[2],set_message[3],set_message[4]);

    unsigned char ua_message[5];
    ua_message[0] = FLAG_RCV;
    ua_message[1] = A_RCV;
    ua_message[2] = UA;
    ua_message[3] = A_RCV ^ C_RCV;
    ua_message[4] = FLAG_RCV;
    write(fd, ua_message, 5);
    printf("Sent: %x %x %x %x %x\n", ua_message[0], ua_message[1],ua_message[2],ua_message[3],ua_message[4]);
}


////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters)
{
    fd = open(connectionParameters.serialPort, O_RDWR | O_NOCTTY);
    
    if (fd < 0)
    {
        perror(connectionParameters.serialPort);
        exit(-1);
    }

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
    newtio.c_cc[VTIME] = 30; // Inter-character timer unused
    newtio.c_cc[VMIN] = 0;  // Blocking read until 5 chars received

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

    numTries = connectionParameters.nRetransmissions;
    timeout = connectionParameters.timeout;

    printf("New termios structure set\n");
   
    if (connectionParameters.role == LlTx){
        role = LlTx;
        llopen_t();
    }
    else
    {
        role = LlRx;
        llopen_r();
    }

    
    return 1;
    
}

unsigned char stuff(unsigned char * block){

    unsigned char res = 0x00;
    if (*block == FLAG){
        *block = *block ^ STUFFER;
        res |= ESC;
        res |= (*block & 0x00FF);
    }
    else if (*block == ESC){
        res |= (*block ^ STUFFER);
    }

    return res;

}

unsigned char destuff(unsigned char * block){

    unsigned char res= 0x00;
    res |= (*block ^STUFFER);
    return res;
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const unsigned char *buf, int bufSize, unsigned char *frame_to_send)
{   
    
    unsigned char buf_to_stuff[bufSize];

    unsigned char BCC2 = 0x00;
    for (int i = 4; i < 4 + bufSize; i++){
        unsigned char temp_packet = buf[i-4];
        buf_to_stuff[i-4] = stuff(&temp_packet);
        BCC2 ^= buf_to_stuff[i-4];
    }

    frame_to_send[0] = FLAG;
    frame_to_send[1] = A_RCV;
    frame_to_send[2] = 0x00 ^ (S << 6);
    frame_to_send[3] = frame_to_send[1] ^ frame_to_send[2];

    frame_to_send[bufSize + 2] = BCC2;
    frame_to_send[bufSize + 3] = FLAG;

    for (int i = 4; i < 4+ bufSize; i++){
        frame_to_send[i-4] = buf_to_stuff[i-4];
    }

    int bytes_written = write(fd, frame_to_send, sizeof(buf_to_stuff) + 3);

    if (bytes_written < 0){
        return -1;
    }
    S = S^1;
    return bytes_written;

/*

int has_received = FALSE;

    if (buf == NULL || bufSize < 0){
        return -1;
    }

    if (bufSize > MAX_PAYLOAD_SIZE){
        return -1;
    }

    (void)signal(SIGALRM, alarmHandler);

    unsigned char buf1[bufSize + 7];

    for (int i = 4; i < 4+ bufSize; i++){
        buf1[i-4] = buf[i-4];
    }

    int BCC_2 = 0x00;  
    buf1[0] = FLAG_RCV;
    buf1[1] = A_T;
    buf1[2] = C_T;
    buf1[3] = A_T ^ C_T;
    buf1[bufSize + 3] = BCC_2; 
    buf1[bufSize + 4] = FLAG_RCV;
    for (int i = 4; i < 4 + bufSize; i++){
        BCC_2 = BCC_2 ^ buf1[i-1];
        printf("%x\n", buf1[i-4]);
    }
    for (int i = 4; i < bufSize; i++){
        buf1[i] = stuff(&buf1[i]);

    }
    int send_count = 0;
    unsigned char flag_received[1] = {0};


    while(!has_received && alarmCount < 4){

        send_count = write(fd, buf1, bufSize + 6);

        sleep(1);
        alarm(3);

        alarmEnabled = TRUE;

        while (state != STOP_){
            state = START; 
            flag_received[0] = read(fd, flag_received, 1);
            set_stateT(state, flag_received[0]);
        }

    }

    if (!has_received) return -1;

    return (send_count - 6);
*/

}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet)
{
    
    if (packet == NULL){
        return -1;
    }
    unsigned char destuffed;
    destuffed = destuff(packet);

    if (packet == NULL){
        return 0;
    }
    
    unsigned char flag_received[1];
    int frames_received = read(fd,flag_received, 1);
    set_stateR(fd,flag_received[0]);

    unsigned char accepted[5];
    accepted[0] = FLAG;
    accepted[1] = A_RCV;
    accepted[2] = C_RCV;
    accepted[3] = A_RCV ^ C_RCV;
    accepted[4] = FLAG;
    int bytes_sent = write(fd, accepted, 5);
    printf("%d bytes accepted written\n", bytes_sent);
    memcpy(packet, &destuffed, sizeof(packet) + 6);
    return (frames_received+4);
    
    
}
////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////


void llclose_t()
{
    unsigned char closeFrame[5];

    closeFrame[0] = FLAG;
    closeFrame[1] = A;
    closeFrame[2] = C_DISC;
    closeFrame[3] = A ^ C_DISC;
    closeFrame[4] = FLAG;

    //int bytesReceived = 0;
    int bytesDTransmitted = write(fd,closeFrame, 5);

    printf("%d Bytes in close written\n", bytesDTransmitted);
    
    //suposedly state machine

    unsigned char endFrame[5];

    endFrame[0] = FLAG;
    endFrame[1] = A_RCV;
    endFrame[2] = UA;
    endFrame[3] = A_RCV ^ UA;
    endFrame[4] = FLAG;

    int bytesUATransmitted = write(fd,endFrame,5);
    printf("%d Bytes in End written\n", bytesUATransmitted);


}

void llclose_r()
{
    unsigned char closeFrameR[5];
    closeFrameR[0] = FLAG;
    closeFrameR[1] = A_RCV;
    closeFrameR[2] = C_DISC;
    closeFrameR[3] = A_RCV ^ C_DISC;
    closeFrameR[4] = FLAG;

    int bytesReceivedR = write (fd, closeFrameR, 5);
    printf("%d Bytes in closeR written\n", bytesReceivedR);
    
}

int llclose(int showStatistics)
{

    if (showStatistics == 1){
        printf("====Statistics====\n");
        printf("Number of retransmitted frames: %d\n", total_retransmits);
        printf("Number of received I frames: %d\n", total_received_frames);
        printf("Number of timeouts: %d\n", total_timeouts);
        printf("Number of sent REJs: %d\n", total_rej);
    }

    if (tcsetattr(fd, TCSANOW, &oldtio) == -1){
        return -1;
    }
     
    close(fd);
     
    return 1;
}
