#include "transmit.h"
#include "uart.h"
#include "valve.h"
#include "main.h"
#include "auto_control.h"
#include "can.h"

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;

extern FDCAN_HandleTypeDef hfdcan2;
extern FDCAN_TxHeaderTypeDef ERROR_CAN;

Alarm_Status Alarm = NO_ALARM;
static uint32_t fault_start_time = 0;
extern float Pressure;
extern float Voltage;
extern int Temp;
extern int Current;
extern int Entry;
extern uint32_t Time;
int cont_print = 0;
int Alarm_Entry = 1;
int inteiro;
int decimal;

void H2_reaction_tracker(void)
{

    H2_reacted = H2_REACTION_PER_SEC_A * Current * PurgeLastCallTime;

    if (H2_consumed + H2_reacted >= H2_TOTAL_MASS) {
        H2_consumed = H2_TOTAL_MASS;
    } else {
        H2_consumed += H2_reacted;
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
                HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &ERROR_CAN, ERRO_C);
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
                HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &ERROR_CAN, ERRO_C);
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
                HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &ERROR_CAN, ERRO_C);
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
                HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &ERROR_CAN, ERRO_C);
            }
            break;

        case NO_ALARM:
            return;
        }
    }
}

void print_sensors(int print_sensor, int print_state)
{
    inteiro = (int)Pressure;
    decimal = (int)((Pressure - inteiro) * 100);

    H2_reaction_tracker();

    if (cont_print >= 10) {
        if (print_sensor) {
            print("Temp: %dC; ",          (int)Temp);
            print("Current: %dA; ",       (int)Current);
            print("Voltage: %dV; ",       (int)Voltage);
            print("Pressure: %d.%02d; ",  inteiro, decimal);
            print("FAN: %d%%; ",          Fan_power);
            print("H2: %dg; ",            (int)H2_consumed);
            print("A*s: %d; ",            (int)amp_segundo);
            print("Purgas: %d; ",         purgas);
            println("Estado: %d",         (int)FcActualState);
        }
        cont_print = 0;
    }

    Check_Alarms();
    cont_print++;
}
