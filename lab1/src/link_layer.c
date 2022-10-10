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

    return 1;
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const unsigned char *buf, int bufSize)
{
    // TODO

    return 0;
}

int stuff(int block){

    int res = 0x0000;
    if (block == FLAG){
        block = block ^ STUFFER;
        res |= ESC;
        res |= (block & 0x00FF);
    }
    else if (block == ESC){
        res |= (block ^ STUFFER);
    }

    return res;

}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet)
{
    

    return 0;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose(int showStatistics)
{
    // TODO

    return 1;
}
