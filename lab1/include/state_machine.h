#ifndef _STATE_MACHINE_H_
#define _STATE_MACHINE_H_

#define START 0
#define STATE_FLAG_RCV 1
#define A_RCV 2
#define C_RCV 3
#define BCC_OK 4
#define STOP_ 6

#define FLAG_RCV 0b01111110

void set_state(int state, int flag);

#endif
