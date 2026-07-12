#include "alarms.h"
#include "context.h"
#include "uart.h"
#include "valve.h"
#include "main.h"
#include "auto_control.h"
#include "can.h"

/* --- HANDLES EXTERNOS (ADC / CAN) --- */
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;

extern FDCAN_HandleTypeDef   hfdcan2;
extern FDCAN_TxHeaderTypeDef ERROR_CAN;
extern FDCAN_TxHeaderTypeDef CHECK;

/* --- VARIÁVEIS LOCAIS DE ALARME --- */
Alarm_Status Alarm             = NO_ALARM;
static uint32_t fault_start_time = 0;
float    Current_To_Voltage    = 0;
uint32_t fault                 = 0;
uint32_t last_reading           = 0;
int      Alarm_Entry           = 1;

/* --- VERIFICAÇÃO DE TENSÃO NOMINAL --- */
void CheckVoltageNominal(void)
{
    if (Current < 7.5)
    {
        Current_To_Voltage = -1.04 * Current + 46.0;
    }
    else
    {
        Current_To_Voltage = -0.147 * Current + 39.3;
    }

    fault = (Voltage < (Current_To_Voltage - 10.0));

    if (fault)
    {
        if (Time - last_reading > 1000)
        {
            Entry = 1;
            FcActualState = FC_ALARM;
            ERRO_C[2] = 1;
        }
    }
    else
    {
        last_reading = Time;
    }
}

/* --- VERIFICAÇÃO DE ALARMES (temperatura, pressão, corrente, tensão) --- */
void Check_Alarms(void)
{
    if (Alarm_Entry == 1)
    {
        // Detecta a primeira condição de falha e memoriza o instante inicial
        if (Temp >= MAX_TEMP || Temp <= MIN_TEMP)
        {
            fault_start_time = Time;
            Alarm = ALARM_TEMP;
            Alarm_Entry = 0;
        }
        else if (Pressure >= MAX_PRESSURE ||
                 (Pressure <= MIN_PRESSURE &&
                  (FcActualState == FC_STARTUP_STARTUPPURGE ||
                   FcActualState == FC_WARMUP ||
                   FcActualState == FC_RUN)))
        {
            fault_start_time = Time;
            Alarm = ALARM_PRESSURE;
            Alarm_Entry = 0;
        }
        else if (Current >= MAX_CURRENT || Current <= MIN_CURRENT)
        {
            fault_start_time = Time;
            Alarm = ALARM_CURRENT;
            Alarm_Entry = 0;
        }
        else if (Voltage >= MAX_VOLTAGE || Voltage <= MIN_VOLTAGE)
        {
            fault_start_time = Time;
            Alarm = ALARM_VOLTAGE;
            Alarm_Entry = 0;
        }
    }
    else
    {
        // Já em condição de falha: verifica se persiste até o tempo limite
        switch (Alarm)
        {
            case ALARM_TEMP:
                if (!(Temp >= MAX_TEMP || Temp <= MIN_TEMP))
                {
                    Alarm = NO_ALARM;
                    Alarm_Entry = 1;
                    break;
                }
                if (Time - fault_start_time >= ALARM_TIME_TEMP)
                {
                    Entry = 1;
                    FcActualState = FC_ALARM;
                    ERRO_C[0] = 1;
                }
                break;

            case ALARM_PRESSURE:
                if (!(Pressure >= MAX_PRESSURE ||
                      (Pressure <= MIN_PRESSURE &&
                       (FcActualState == FC_STARTUP_STARTUPPURGE || FcActualState == FC_RUN))))
                {
                    Alarm = NO_ALARM;
                    Alarm_Entry = 1;
                    break;
                }
                if (Time - fault_start_time >= ALARM_TIME_PRESSURE)
                {
                    Entry = 1;
                    FcActualState = FC_ALARM;
                    ERRO_C[3] = 1;
                }
                break;

            case ALARM_CURRENT:
                if (!(Current >= MAX_CURRENT || Current <= MIN_CURRENT))
                {
                    Alarm = NO_ALARM;
                    Alarm_Entry = 1;
                    break;
                }
                if (Time - fault_start_time >= ALARM_TIME_CURRENT)
                {
                    Entry = 1;
                    FcActualState = FC_ALARM;
                    ERRO_C[1] = 1;
                }
                break;

            case ALARM_VOLTAGE:
                if (!(Voltage >= MAX_VOLTAGE || Voltage <= MIN_VOLTAGE))
                {
                    Alarm = NO_ALARM;
                    Alarm_Entry = 1;
                    break;
                }
                if (Time - fault_start_time >= ALARM_TIME_VOLTAGE)
                {
                    Entry = 1;
                    FcActualState = FC_ALARM;
                    ERRO_C[2] = 1;
                }
                break;

            case NO_ALARM:
                return;
        }
    }
}

/* --- ENVIO DE MENSAGEM CAN --- */
void SEND_CAN_Message(void)
{
    Slice_DATA();
    HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &CHECK, DATA);
    HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &ERROR_CAN, ERRO_C);
}
