#ifndef CONTEXT_H
#define CONTEXT_H

#include "main.h"   // Garante que tipos como uint32_t e HAL_StatusTypeDef funcionem

/* --- LIMITES DA FAN --- */
#define FAN_MAX_CMD   100
#define FAN_MIN_CMD   10

/* --- DURAÇÃO DOS ESTADOS (ms) --- */
#define FAN_SPOOLUP_DURATION    3000
#define FAN_SU_PURGE_DURATION   200
#define WARMUP_DURATION         12000
#define SHUTDOWN_DURATION       180000

/* --- CONTROLE DE PURGA --- */
#define RESET_TIME        3000
#define PURGE_INTERVAL    2300   // não é tempo, é amp * s
#define PURGE_DURATION     300

/* --- CONSTANTES DO PID (controle da fan) --- */
#define P_CONST 8
#define I_CONST 0
#define D_CONST 0

/* --- ENUMS (TIPOS DE DADOS) --- */
typedef enum {
    CLOSED = 0,
    OPEN   = 1
} Valve_Status;

typedef enum {
    FC_STANDBY              = 0x00U,
    FC_STARTUP_FANSPOOLUP   = 0x01U,
    FC_STARTUP_STARTUPPURGE = 0x02U,
    FC_WARMUP               = 0x03U,
    FC_STARTUP_END          = 0x04U,
    FC_RUN                  = 0x05U,
    FC_SHUTDOWN             = 0x06U,
    FC_ALARM                = 0x07U
} Fc_State;

typedef enum {
    ALARM_TEMP     = 0x00U,
    ALARM_PRESSURE = 0x01U,
    ALARM_CURRENT  = 0x02U,
    ALARM_VOLTAGE  = 0x03U,
    NO_ALARM       = 0x04U
} Alarm_Status;

/* --- LIMITES DE SEGURANÇA --- */
#define MAX_TEMP             75
#define MIN_TEMP            -10
#define MAX_PRESSURE          0.69f
#define MIN_PRESSURE          0.07f
#define MAX_CURRENT           78
#define MIN_CURRENT           -3
#define MAX_VOLTAGE           50.6f
#define MIN_VOLTAGE          -10.0f
#define ALARM_TIME_TEMP     5000
#define ALARM_TIME_PRESSURE 1000
#define ALARM_TIME_CURRENT  2000
#define ALARM_TIME_VOLTAGE  5000

/* --- VARIÁVEIS GLOBAIS: ESTADO DO SISTEMA --- */
extern Alarm_Status Alarm_Type;
extern Fc_State      FcActualState;
extern int           Entry;

/* --- VARIÁVEIS GLOBAIS: SENSORES --- */
extern float    Voltage;
extern int      Temp;
extern int      Current;
extern float    Pressure;
extern uint32_t Time;
extern int      Fan_power;

/* --- CONSUMO DE HIDROGÊNIO --- */
#define H2_TOTAL_MASS         73.0     // total de hidrogênio em g
#define H2_PURGE_CONSUMPTION   0.07728 // gasto de uma purga em g
#define H2_REACTION_PER_SEC_A   0.0004784 // gasto por segundo por amp em g*s^-1*A^-1

extern uint32_t last_H2_calc;
extern float    H2_reacted;
extern float    delta_t;
extern float    H2_consumed;
extern float    amp_segundo;
extern int      purgas;

/* --- VARIÁVEIS GLOBAIS: PROTOCOLO CAN --- */
extern uint32_t MAILBOX;
extern uint8_t  DATA[8];
extern uint8_t  ERRO_C[8];
extern uint8_t BMS[8];

/* --- STATUS DOS ATUADORES --- */
extern Valve_Status PurgeValveStatus;
extern Valve_Status SupplyValveStatus;
extern Valve_Status ResistorContactorStatus;
extern Valve_Status MainContactorStatus;

/* --- VARIÁVEIS DE CONTROLE --- */
extern float    AmpSecSincePurge;
extern uint32_t PurgeLastCallTime;
extern uint32_t PurgeStartTime;
extern uint32_t FanLastCallTime;
extern float    LastITerm;
extern int      LastTempDiff;
extern float    PreviousFanCmd;
extern uint32_t fanspoolup_start_time;
extern uint32_t startuppurge_start_time;
extern uint32_t warmup_start_time;
extern uint32_t shutdown_start_time;

#endif // CONTEXT_H
