#include "fan.h"
#include "main.h"

void Fan_CMD(int value){
    if(value < 0){
        value = 0;
    } else if(value > 100){
        value = 100;
    }
    // Certifique-se que TIM1 está inicializado no main.c
    TIM1->CCR1 = value;
    Fan_power = value;
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, value);
}
