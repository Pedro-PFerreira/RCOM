#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include "../include/state_machine.h"
#include "../include/macros.h"

void set_stateR(int fd, unsigned char flag){
    int state = START;
    while(state != STOP_){
        switch (state)
        {
        case START:
            if (fd == FLAG_RCV){
                state = STATE_FLAG_RCV;            
            }
            else{
                state = START;
            }
            break;

        case STATE_FLAG_RCV:
            if (fd == FLAG_RCV){
                state = STATE_FLAG_RCV;
            }
            else if (fd == A_RCV){
                state = A_RCV;
            }
            else{
                state = START;
            }
            break;

        case A:
            if (fd == flag)
            {
                state = C_RCV;
            }
            else if (fd == FLAG_RCV)
            {
                state = STATE_FLAG_RCV;
            }
            else
            {
                state = START;
            }
            break;
            
        case C:
            if ((A_RCV ^ C_RCV) == BCC_OK){
                state = BCC_OK;
            }
            else if(fd == FLAG_RCV)
            {
                state = STATE_FLAG_RCV;
            }
            else{
                state = START;
            }
            break;

        case BCC_OK:
            if (fd == FLAG_RCV){
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

void set_stateT(int state, unsigned char flag){
    switch (state)
        {
        case START:
            if (flag== FLAG_RCV){
                state = STATE_FLAG_RCV;            
            }
            break;

        case STATE_FLAG_RCV:
            if (flag == FLAG){
                state = STATE_FLAG_RCV;
            }
            else if (flag == A_RCV){
                state = A;
            }
            else{
                state = START;
            }
            break;

        case A:
            if (flag == UA)
            {
                state = C_RCV;
            }
            else if (flag == FLAG)
            {
                state = STATE_FLAG_RCV;
            }
            else
            {
                state = START;
            }
            break;
            
        case C:
            if ((A_RCV ^ C_RCV) == BCC_OK){
                state = BCC_OK;
            }
            else if(flag == FLAG)
            {
                state = STATE_FLAG_RCV;
            }
            else{
                state = START;
            }
            break;

        case BCC_OK:
            if (flag == FLAG){
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
