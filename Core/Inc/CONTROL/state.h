#ifndef STATE_H
#define STATE_H

#include "context.h"

void Run_State_Machine(void);

void FC_StandBy_State(void);
void FC_StartUp_FanSpoolUp_State(void);
void FC_StartUp_StartUpPurge_State(void);
void FC_StartUp_End_State(void);
void FC_WarmUp(void);
void FC_Run_State(void);
void FC_Shutdown_State(void);
void FC_Alarm_State(void);

#endif
