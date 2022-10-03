
#define START 0
#define STATE_FLAG_RCV 1
#define A_RCV 2
#define C_RCV 3
#define BCC_OK 4
#define STOP 6

#define FLAG_RCV 0x01111110

void set_state(int state, int flag){
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
            state = STOP;
        }
        else
        {
            state = START;
        }
        break;
    
    case STOP:
        break;
    default:
     break;
    }

}

