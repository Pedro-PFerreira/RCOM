#include "state_machine.h"
#include "macros.h"

void set_state_r(int fd, unsigned char flag){
    unsigned char f;
    int state = START;
    while(state != STOP_){
        read(fd, &f, 1);
        switch (state)
        {
        case START:
            if (f == FLAG_RCV){
                state = STATE_FLAG_RCV;            
            }
            else{
                state = START;
            }
            break;

        case STATE_FLAG_RCV:
            if (f == FLAG_RCV){
                state = STATE_FLAG_RCV;
            }
            else if (f == A_RCV){
                state = A_RCV;
            }
            else{
                state = START;
            }
            break;

        case A:
            if (f == flag)
            {
                state = C_RCV;
            }
            else if (f == FLAG_RCV)
            {
                state = STATE_FLAG_RCV;
            }
            else
            {
                state = START;
            }
            break;
            
        case C:
            if (A_RCV ^ C_RCV == BCC_OK){
                state = BCC_OK;
            }
            else if(f == FLAG_RCV)
            {
                state = STATE_FLAG_RCV;
            }
            else{
                state = START;
            }
            break;

        case BCC_OK:
            if (f == FLAG_RCV){
                state = STOP_;
            }
            else
            {
                state = START;
            }
            break;
        
        case STOP_:
            break;
        default:
        break;
        }        
    }
}

void set_state_machineT(int *state, unsigned char* flag){
    switch (*state)
        {
        case START:
            if (*flag== FLAG_RCV){
                state = STATE_FLAG_RCV;            
            }
            break;

        case STATE_FLAG_RCV:
            if (*flag == FLAG){
                state = STATE_FLAG_RCV;
            }
            else if (*flag == A_RCV){
                state = A;
            }
            else{
                state = START;
            }
            break;

        case A:
            if (*flag == UA)
            {
                state = C_RCV;
            }
            else if (*flag == FLAG)
            {
                state = STATE_FLAG_RCV;
            }
            else
            {
                state = START;
            }
            break;
            
        case C:
            if (A_RCV ^ C_RCV == BCC_OK){
                state = BCC_OK;
            }
            else if(*flag == FLAG)
            {
                state = STATE_FLAG_RCV;
            }
            else{
                state = START;
            }
            break;

        case BCC_OK:
            if (*flag == FLAG){
                state = STOP_;
            }
            else
            {
                state = START;
            }
            break;
        
        case STOP_:
            break;
        default:
        break;
        }  
}