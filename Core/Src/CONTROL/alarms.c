#include "alarms.h"
#include "uart.h"
#include "valve.h"
#include "main.h"
#include "auto_control.h"
#include "can.h"

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;

extern FDCAN_HandleTypeDef hfdcan2;
extern FDCAN_TxHeaderTypeDef ERROR_CAN;
extern FDCAN_TxHeaderTypeDef CHECK;

Alarm_Status Alarm = NO_ALARM;
static uint32_t fault_start_time = 0;
float Current_To_Voltage = 0;
uint32_t fault = 0;
uint32_t last_reading = 0;
extern float Pressure;
extern float Voltage;
extern int Temp;
extern int Current;
extern int Entry;
extern uint32_t Time;
int Alarm_Entry = 1;

void CheckVoltageNominal() {
        if (Current < 8){
            Current_To_Voltage = (-0.01875*Current + 1) * 46;
        }else{
            Current_To_Voltage =  (-0.002*Current + 0.83) * 46;
        }
        fault = (Voltage < Current_To_Voltage - 10);
        if ((Time - fault > 1000) && fault) {
        	Entry = 1;
        	FcActualState = FC_ALARM;
        	ERRO_C[2] = 1;
        }else if (!fault) {
            last_reading = Time;
        }
}

void Check_Alarms(void)
{
    if (Alarm_Entry == 1) {
        if (Temp >= MAX_TEMP || Temp <= MIN_TEMP) {
            fault_start_time = Time;
            Alarm       = ALARM_TEMP;
            Alarm_Entry = 0;
        }
        else if (Pressure >= MAX_PRESSURE ||
                 (Pressure <= MIN_PRESSURE &&
                  (FcActualState == FC_STARTUP_STARTUPPURGE || 
                    FcActualState == FC_WARMUP ||
                    FcActualState == FC_RUN))) {
            fault_start_time = Time;
            Alarm       = ALARM_PRESSURE;
            Alarm_Entry = 0;
        }
        else if (Current >= MAX_CURRENT || Current <= MIN_CURRENT) {
            fault_start_time = Time;
            Alarm       = ALARM_CURRENT;
            Alarm_Entry = 0;
        }
        else if (Voltage >= MAX_VOLTAGE || Voltage <= MIN_VOLTAGE) {
            fault_start_time = Time;
            Alarm       = ALARM_VOLTAGE;
            Alarm_Entry = 0;
        }
    }
    else {
        switch (Alarm) {
        case ALARM_TEMP:
            if (!(Temp >= MAX_TEMP || Temp <= MIN_TEMP)) {
                Alarm = NO_ALARM; Alarm_Entry = 1; break;
            }
            if (Time - fault_start_time >= ALARM_TIME_TEMP) {
            	Entry = 1;
                FcActualState = FC_ALARM;
                ERRO_C[0] = 1;
            }
            break;

        case ALARM_PRESSURE:
            if (!(Pressure >= MAX_PRESSURE ||
                  (Pressure <= MIN_PRESSURE &&
                   (FcActualState == FC_STARTUP_STARTUPPURGE || FcActualState == FC_RUN)))) {
                Alarm = NO_ALARM; Alarm_Entry = 1; break;
            }
            if (Time - fault_start_time >= ALARM_TIME_PRESSURE) {
            	Entry = 1;
                FcActualState = FC_ALARM;
                ERRO_C[3] = 1;
            }
            break;

        case ALARM_CURRENT:
            if (!(Current >= MAX_CURRENT || Current <= MIN_CURRENT)) {
                Alarm = NO_ALARM; Alarm_Entry = 1; break;
            }
            if (Time - fault_start_time >= ALARM_TIME_CURRENT) {
            	Entry = 1;
                FcActualState = FC_ALARM;
                ERRO_C[1] = 1;
            }
            break;

        case ALARM_VOLTAGE:
            if (!(Voltage >= MAX_VOLTAGE || Voltage <= MIN_VOLTAGE)) {
                Alarm = NO_ALARM; Alarm_Entry = 1; break;
            }
            if (Time - fault_start_time >= ALARM_TIME_VOLTAGE) {
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

void SEND_CAN_Message(void) {
  Slice_DATA();
  HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &CHECK, DATA);
  HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &ERROR_CAN, ERRO_C);
}
