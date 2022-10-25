#ifndef _STATE_MACHINE_H_
#define _STATE_MACHINE_H_

#define START 0
#define STATE_FLAG_RCV 1
#define A 2
#define C 3
#define BCC_OK 4
#define STOP_ 5

void set_state(int sp, unsigned char flag);

void set_stateT(int * state, unsigned char flag);

#endif
