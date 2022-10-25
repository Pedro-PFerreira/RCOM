#ifndef _ALARM_H_
#define _ALARM_H_

#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include "macros.h"

extern int alarmEnabled;
extern int alarmCount;

extern int RECEIVED;

// Alarm function handler
void alarmHandler(int signal);

int createAlarm();

#endif
