#ifndef STATE_H
#define STATE_H

#include "context.h"

void FC_StandBy_State(void);
void FC_StartUp_FanSpoolUp_State(void);
void FC_StartUp_StartUpPurge_State(void);
void FC_StartUp_End_State(void);
void FC_WarmUp(void);
void FC_Run_State(void);
void FC_Shutdown_State(void);
void FC_Alarm_State(void);
int round_p(float value);
int StartECU(void);
void ForceSupply(void);
void ForcePurge(void);
void Air_Starve(void);

#endif
