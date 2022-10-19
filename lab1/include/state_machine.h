#ifndef _STATE_MACHINE_H_
#define _STATE_MACHINE_H_

#define START 0
#define STATE_FLAG_RCV 1
#define A_RCV 0x03
#define A_T 0x01
#define C_T 5
#define C_RCV 6
#define BCC_OK 7
#define STOP_ 8

void set_state(int state, int flag);

#endif
