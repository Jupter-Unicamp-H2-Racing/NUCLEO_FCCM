#include "contactor.h"
#include "main.h" // Necessário para HAL_GPIO_WritePin e definições de pinos

void ResistorContactor_CMD(Valve_Status status){
    if(status == OPEN){
        HAL_GPIO_WritePin(RESISTOR_GPIO_Port, RESISTOR_Pin, GPIO_PIN_RESET);
        ResistorContactorStatus = OPEN;
    } else if (status == CLOSED){
        HAL_GPIO_WritePin(RESISTOR_GPIO_Port, RESISTOR_Pin, GPIO_PIN_SET);
        ResistorContactorStatus = CLOSED;
    }
}

void MainContactor_CMD(Valve_Status status){
    if(status == OPEN){
        HAL_GPIO_WritePin(MAIN_GPIO_Port, MAIN_Pin, GPIO_PIN_RESET);
        MainContactorStatus = OPEN;
    } else if (status == CLOSED){
        HAL_GPIO_WritePin(MAIN_GPIO_Port, MAIN_Pin, GPIO_PIN_SET);
        MainContactorStatus = CLOSED;
    }
}
