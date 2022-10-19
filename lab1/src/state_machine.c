#include "state_machine.h"
#include "macros.h"

void set_state_r(int state, int flag){

    while(state != STOP_){
        
        switch (state)
        {
        case START:
            if (flag == FLAG_RCV){
                state = STATE_FLAG_RCV;            
            }
            else{
                state = START;
            }
            break;

        case STATE_FLAG_RCV:
            if (flag == FLAG_RCV){
                state = STATE_FLAG_RCV;
            }
            else if (flag == A_RCV){
                state = A_RCV;
            }
            else{
                state = START;
            }
            break;

        case A_RCV:
            if (C_RCV == 1)
            {
                state = C_RCV;
            }
            else if (flag == FLAG_RCV)
            {
                state = STATE_FLAG_RCV;
            }
            else
            {
                state = START;
            }
            break;
            
        case C_RCV:
            if (A_RCV ^ C_RCV == BCC_OK){
                state = BCC_OK;
            }
            else if(flag == FLAG_RCV)
            {
                state = STATE_FLAG_RCV;
            }
            else{
                state = START;
            }
            break;

        case BCC_OK:
            if (flag == FLAG_RCV){
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

