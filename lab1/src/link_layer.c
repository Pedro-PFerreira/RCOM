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

unsigned char set_message[5];
unsigned char ua[5];

int llopen_t(){

    set_message[0] = FLAG_RCV;
    set_message[1] = A_T;
    set_message[2] = SET;
    set_message[3] = A_T ^ C_T;
    set_message[4] = FLAG_RCV;
    if (write(fd, set_message, 5) > 0){
        while (total_retransmits <= 3){
            if (read(fd, &ua, 6) < 0 && total_retransmits < 3){
                total_frames_sent++;
                write(fd, set_message, 5);
            }
            else if(total_retransmits == 3){
                return -1;
            }
            else{
                total_retransmits = 0;
                return 0;
            }       
        }        
    }
    return 1;

}

int llopen_r()
{
    ua[0] = FLAG_RCV;
    ua[1] = A_RCV;
    ua[2] = UA;
    ua[3] = A_RCV ^ C_RCV;
    ua[4] = FLAG_RCV;
    
    if (read(fd, &set_message, 6) >= 0){

        bytes = write(fd, ua, 5);
        if(bytes < 0 && total_retransmits < 3) 
        {
            write(fd, ua, 5);
        }
        else if(total_retransmits == 3){
            return -1;
        }
        else{
            total_retransmits = 0;
            return 0;
        }      
    }

    printf("Sent: %s:%d\n", send_buf, bytes);

    return 1;
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
        int res = llopen_t();
    }
    else
    { // if (connectionParameters.role == LlRx){
        role = LlRx;
        int res = llopen_r();
    }

    //printf("Sent: %s:%d\n", send_buf, bytes);
    // The while() cycle should be changed in order to respect the specifications
    // of the protocol indicated in the Lab guide



    // Restore the old port settings
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
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
int llwrite(const unsigned char *buf, int bufSize)
{   

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
int llclose(int showStatistics)
{

    if (role == LlTx){
       unsigned char* frame_disc[5], frame_ua[5];
        int num_send_frame = 0;
        timeout = TRUE;
        int received_disc = FALSE;
        ////createAlarm();
        do{
            if (write(fd, frame_disc, 5) == -1){
                return -1;
            }
            num_send_frame++;
            timeout = FALSE;
            ////alarm(num_send_frame);

            if (read(fd, &frame_disc, 5) == 0){
                received_disc = TRUE;
            }

        }while(num_send_frame < numTries|| !timeout); 
        
        ////alarm(0);

        if (!received_disc){
            if (tcsetattr(fd, TCSANOW, &oldtio) == -1){           
                return -1;
            }
            set_stateT(fd, (unsigned char) STOP_);
            return -1;
        }   
        if (write(fd, &frame_ua,5) == -1){
            //set_stateT(&fd, (unsigned char) STOP_);
            return -1;
        }
    }

    else if (role == LlRx){
        unsigned char* frame_disc[5], frame_ua[5];
        int num_send_frame = 0;
        timeout = FALSE;
        int read_r;
        //createAlarm();

        do{
            read_r = read(fd,&frame_disc, 5);
            if (read_r == -1){continue;}
            if (*frame_disc[1] == A_RCV && *frame_disc[2] == C_RCV){
                if (*frame_disc == NULL){
                    total_received_frames++;
                }
                if(write(fd, frame_disc, 5) == -1){
                    
                    return -1;
                } 
                total_received_frames++;
                continue;               
            }
            if(timeout == TRUE){
                //alarm(0);
                return -1;
            }

        } while(!timeout);

        do{

            if (timeout && num_send_frame < numTries){
                if (write(fd, frame_disc,5) == -1){
                    return -1;
                }
            }
            if (num_send_frame != 0){
                total_timeouts++;
                total_retransmits++;
            }
            num_send_frame++;
            timeout = FALSE;
            //alarm(timeout);
            read(fd,&frame_ua, 5);
        }while(num_send_frame < numTries);

        //alarm(0);
    }
    
    else{return -1;}
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1){
        return -1;
    }
    //set_stateT(&fd, STOP_);

    if (showStatistics == 1){
            printf("====Statistics====\n");
            printf("Number of retransmitted frames: %d\n", total_retransmits);
            printf("Number of received I frames: %d\n", total_received_frames);
            printf("Number of timeouts: %d\n", total_timeouts);
            printf("Number of sent REJs: %d\n", total_rej);
        }

    return 1;
}
