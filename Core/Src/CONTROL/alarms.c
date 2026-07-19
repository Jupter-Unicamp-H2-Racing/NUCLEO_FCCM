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
extern FDCAN_TxHeaderTypeDef BMS_CAN;

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
    float nominal_voltage;
    float low_limit_voltage;

    if (Current <= 65.0f)
    {
        // Curva nominal
        nominal_voltage = -0.20f * Current + 45.5f;

        // Limite inferior (Lo Limit)
        low_limit_voltage = 30;
    }
    else
    {
        // Curva nominal
        nominal_voltage = -0.12f * Current + 40.3f;

        // Limite inferior
        low_limit_voltage = -0.15f * Current + 39.75f;
    }

    Current_To_Voltage = nominal_voltage;

    fault = (Voltage < low_limit_voltage);

    if (fault)
    {
        if ((Time - last_reading) > 1000)
        {
            Entry = 1;
            FcActualState = FC_ALARM;
            ERRO_C[2] = 1;
            BMS[0] = 2;
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
                    BMS[0] = 2;
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
                    BMS[0] = 2;
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
                    BMS[0] = 2;
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
                    BMS[0] = 2;
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
    HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &BMS_CAN, BMS);
    HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &ERROR_CAN, ERRO_C);
}
