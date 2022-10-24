// Alarm example
//
#include <unistd.h>
#include <signal.h>
#include <stdio.h>

#define FALSE 0
#define TRUE 1

int alarmEnabled = FALSE;
int alarmCount = 0;

int RECEIVED = FALSE;

// Alarm function handler
void alarmHandler(int signal)
{
    alarmEnabled = FALSE;

    if (RECEIVED == TRUE)
        return;

    alarmCount++;

    printf("Alarm #%d\n", alarmCount);
}

int createAlarm()
{
    // Set alarm function handler
    (void)signal(SIGALRM, alarmHandler);

    while (alarmCount < 4 && RECEIVED == FALSE)
    {
        if (alarmEnabled == FALSE)
        {
            alarm(3); // Set alarm to be triggered in 3s
            alarmEnabled = TRUE;
            /*
            //write(set);

            //int bytes=read(ua);

            if (bytes>0)
                RECEIVED = TRUE;

*/
        }
    }

    printf("Ending program\n");

    return 0;
}
