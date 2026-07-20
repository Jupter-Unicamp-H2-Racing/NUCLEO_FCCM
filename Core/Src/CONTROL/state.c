#include "state.h"
#include "context.h"
#include "contactor.h"
#include "valve.h"
#include "auto_control.h"
#include "uart.h"
#include "alarms.h"

/* --- VARIÁVEIS EXTERNAS (declaradas em main.c) --- */
extern int Delay_Restart;

/* --- VARIÁVEIS DE CONTROLE DE DELAY INTERNO DOS ESTADOS --- */
int      first_delay_auth     = 1;
int      second_delay_auth    = 0;
uint32_t first_delay_start    = 0;
uint32_t second_delay_start   = 0;
uint32_t startuppurge_start_time;

/* --- MÁQUINA DE ESTADOS PRINCIPAL --- */
void Run_State_Machine(void)
{
    switch (FcActualState)
    {
        case FC_STANDBY:
            FC_StandBy_State();
            break;
        case FC_STARTUP_FANSPOOLUP:
            FC_StartUp_FanSpoolUp_State();
            break;
        case FC_STARTUP_STARTUPPURGE:
            FC_StartUp_StartUpPurge_State();
            break;
        case FC_STARTUP_END:
            FC_StartUp_End_State();
            break;
        case FC_WARMUP:
            FC_WarmUp();
            break;
        case FC_RUN:
            FC_Run_State();
            break;
        case FC_SHUTDOWN:
            FC_Shutdown_State();
            break;
        case FC_ALARM:
            FC_Alarm_State();
            break;
        default:
            FcActualState = FC_STANDBY;
            Entry = 1;
            break;
    }
}

/* --- ESTADO: STANDBY --- */
void FC_StandBy_State(void)
{
    if (Entry == 1)
    {
        Fan_CMD(0);
        SupplyValve_CMD(CLOSED);
        PurgeValve_CMD(CLOSED);
        MainContactor_CMD(OPEN);
        ResistorContactor_CMD(OPEN);
        Entry = 0;
    }
}

/* --- ESTADO: STARTUP - FAN SPOOLUP --- */
void FC_StartUp_FanSpoolUp_State(void)
{
    if (Entry == 1)
    {
        Fan_CMD(FAN_MAX_CMD);
        SupplyValve_CMD(CLOSED);
        PurgeValve_CMD(CLOSED);
        MainContactor_CMD(OPEN);
        ResistorContactor_CMD(OPEN);

        fanspoolup_start_time = Time;
        Entry = 0;
    }
    else
    {
        if (Time - fanspoolup_start_time >= FAN_SPOOLUP_DURATION)
        {
            FcActualState = FC_STARTUP_STARTUPPURGE;
            Entry = 1;
        }
    }
}

/* --- ESTADO: STARTUP - PURGA INICIAL --- */
void FC_StartUp_StartUpPurge_State(void)
{
    if (Entry == 1)
    {
    	H2_reaction_tracker();
        Fan_CMD(FAN_MAX_CMD);
        SupplyValve_CMD(CLOSED);
        PurgeValve_CMD(OPEN);   // conferir
        MainContactor_CMD(OPEN);
        ResistorContactor_CMD(OPEN);

        startuppurge_start_time = Time;
        Entry = 0;
    }
    else
    {
        if (Time - startuppurge_start_time >= FAN_SU_PURGE_DURATION)
        {
        	H2_reaction_tracker();
            FcActualState = FC_WARMUP;
            Entry = 1;
        }
    }
}

/* --- ESTADO: WARMUP --- */
void FC_WarmUp(void)
{
    if (Entry == 1)
    {
    	H2_reaction_tracker();
        Fan_CMD(FAN_MIN_CMD);
        SupplyValve_CMD(OPEN);
        PurgeValve_CMD(CLOSED);
        MainContactor_CMD(OPEN);
        ResistorContactor_CMD(CLOSED);

        warmup_start_time = Time;
        Entry = 0;
    }
    else
    {
        if (Time - warmup_start_time >= WARMUP_DURATION)
        {
        	H2_reaction_tracker();
            FcActualState = FC_STARTUP_END;
            Entry = 1;
        }
    }
}

/* --- ESTADO: FIM DO STARTUP --- */
void FC_StartUp_End_State(void)
{
    if (Entry == 1)
    {
    	H2_reaction_tracker();
        Fan_CMD(FAN_MIN_CMD);
        SupplyValve_CMD(OPEN);
        PurgeValve_CMD(CLOSED);
        MainContactor_CMD(OPEN);
        ResistorContactor_CMD(CLOSED);
        Entry = 0;
    }
    else
    {
        FcActualState = FC_RUN;
        Entry = 1;
    }
}

/* --- ESTADO: RUN (operação normal) --- */
void FC_Run_State(void)
{
    Automatic_Fan_Control();
    CheckVoltageNominal();

    if (Entry == 1)
    {
    	H2_reaction_tracker();
        Automatic_Fan_Control();
        Automatic_Purge_Control();
        SupplyValve_CMD(OPEN);
        MainContactor_CMD(CLOSED);
        ResistorContactor_CMD(OPEN);
        FanLastCallTime = Time;
        PurgeLastCallTime = Time;
        Entry = 0;
    }
    else
    {
    	H2_reaction_tracker();
        Automatic_Fan_Control();
        Automatic_Purge_Control();
        ResistorContactor_CMD(OPEN);
    }
}

/* --- ESTADO: SHUTDOWN --- */
void FC_Shutdown_State(void)
{
    if (Entry == 1)
    {
        Fan_CMD(FAN_MAX_CMD);
        SupplyValve_CMD(CLOSED);
        PurgeValve_CMD(CLOSED);
        MainContactor_CMD(OPEN);
        ResistorContactor_CMD(OPEN);

        shutdown_start_time = Time;
        Entry = 0;
    }
    else
    {
        if (Time - shutdown_start_time > SHUTDOWN_DURATION)
        {
            FcActualState = FC_STANDBY;
            Entry = 1;
            Delay_Restart = 1;
        }
    }
}

/* --- ESTADO: ALARME --- */
void FC_Alarm_State(void)
{
    if (Entry == 1)
    {
        Fan_CMD(0);
        SupplyValve_CMD(CLOSED);
        PurgeValve_CMD(CLOSED);
        ResistorContactor_CMD(OPEN);
        MainContactor_CMD(OPEN);   // garantir que desliga
        Entry = 0;
    }
    // Loop infinito ou aguarda reset manual
}
