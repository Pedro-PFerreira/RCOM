// Link layer protocol implementation

#include "link_layer.h"
#include "state_machine.h"
#include "macros.h"

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters)
{   
    int state = 0;
    set_state(state, FLAG_RCV);

    if (connectionParameters.role == LlRx) return 0;

    else if (connectionParameters.role == LlTx) return 1;

    else return 2;
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const unsigned char *buf, int bufSize)
{
    // TODO

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
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet)
{
    *packet = stuff(packet);

    int packet_sz = *packet / sizeof(unsigned char);

    *packet = destuff(packet);
    
    if (packet_sz > MAX_SIZE) {
        printf("Can't read packet!\n");
        return 1;
    }
    unsigned char buf[packet_sz - 1];

    for (int i = 0; i < packet_sz; i++){
        packet[i]= buf[i];
    }
    

    return packet_sz;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose(int showStatistics)
{
    // TODO

    return 1;
}
