#include "print.h"
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

/* --- VARIÁVEIS LOCAIS --- */
int inteiro;
int decimal;

/* --- IMPRESSÃO DOS SENSORES VIA UART --- */
void print_sensors(int print_sensor, int print_state)
{
    inteiro = (int)Pressure;
    decimal = (int)((Pressure - inteiro) * 100);

    print("Temp: %dC; ",         (int)Temp);
    print("Current: %dA; ",      (int)Current);
    print("Voltage: %dV; ",      (int)Voltage);
    print("Pressure: %d.%02d; ", inteiro, decimal);
    print("FAN: %d%%; ",         Fan_power);
    print("H2: %dg; ",           (int)H2_consumed);
    print("A*s: %d; ",           (int)amp_segundo);
    print("Purgas: %d; ",        purgas);
    println("Estado: %d",        (int)FcActualState);
}
