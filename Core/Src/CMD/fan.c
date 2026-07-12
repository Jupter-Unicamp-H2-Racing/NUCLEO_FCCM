#include "fan.h"
#include "context.h"
#include "main.h"

/* --- COMANDO DA FAN (PWM via TIM1) --- */
void Fan_CMD(int value)
{
    if (value < 0)
    {
        value = 0;
    }
    else if (value > 100)
    {
        value = 100;
    }

    TIM1->CCR1 = value;
    Fan_power = value;
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, value);
}
