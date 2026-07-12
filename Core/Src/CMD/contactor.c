#include "contactor.h"
#include "context.h"
#include "main.h"   // necessário para HAL_GPIO_WritePin e definições de pinos

/* --- CONTATOR DO RESISTOR --- */
void ResistorContactor_CMD(Valve_Status status)
{
    if (status == OPEN)
    {
        HAL_GPIO_WritePin(RESISTOR_GPIO_Port, RESISTOR_Pin, GPIO_PIN_RESET);
        ResistorContactorStatus = OPEN;
    }
    else if (status == CLOSED)
    {
        HAL_GPIO_WritePin(RESISTOR_GPIO_Port, RESISTOR_Pin, GPIO_PIN_SET);
        ResistorContactorStatus = CLOSED;
    }
}

/* --- CONTATOR PRINCIPAL --- */
void MainContactor_CMD(Valve_Status status)
{
    if (status == OPEN)
    {
        HAL_GPIO_WritePin(MAIN_GPIO_Port, MAIN_Pin, GPIO_PIN_RESET);
        MainContactorStatus = OPEN;
    }
    else if (status == CLOSED)
    {
        HAL_GPIO_WritePin(MAIN_GPIO_Port, MAIN_Pin, GPIO_PIN_SET);
        MainContactorStatus = CLOSED;
    }
}
