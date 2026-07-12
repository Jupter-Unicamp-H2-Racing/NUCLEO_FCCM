#include "valve.h"
#include "context.h"
#include "main.h"   // necessário para HAL_GPIO e TIM1

/* --- RASTREADOR DE H2 CONSUMIDO POR REAÇÃO --- */
void H2_reaction_tracker(void)
{
    delta_t = ((float)(Time - last_H2_calc)) / 1000.0f;   // segundos desde a última chamada
    last_H2_calc = Time;

    if (Current > 0)   // só conta reação quando há corrente sendo gerada
    {
        H2_reacted = H2_REACTION_PER_SEC_A * Current * delta_t;

        if (H2_consumed + H2_reacted >= H2_TOTAL_MASS)
        {
            H2_consumed = H2_TOTAL_MASS;
        }
        else
        {
            H2_consumed += H2_reacted;
        }
    }
}

/* --- RASTREADOR DE H2 CONSUMIDO POR PURGA --- */
void H2_purge_tracker(void)
{
    if (H2_consumed + H2_PURGE_CONSUMPTION >= H2_TOTAL_MASS)
    {
        H2_consumed = H2_TOTAL_MASS;
    }
    else
    {
        H2_consumed += H2_PURGE_CONSUMPTION;
    }
}

/* --- VÁLVULA DE SUPRIMENTO --- */
void SupplyValve_CMD(Valve_Status status)
{
    if (status == OPEN)
    {
        HAL_GPIO_WritePin(SUPPLY_GPIO_Port, SUPPLY_Pin, GPIO_PIN_SET);
        SupplyValveStatus = OPEN;
    }
    else if (status == CLOSED)
    {
        HAL_GPIO_WritePin(SUPPLY_GPIO_Port, SUPPLY_Pin, GPIO_PIN_RESET);
        SupplyValveStatus = CLOSED;
    }
}

/* --- VÁLVULA DE PURGA --- */
void PurgeValve_CMD(Valve_Status status)
{
    if (status == OPEN)
    {
        H2_purge_tracker();
        purgas += 1;
        HAL_GPIO_WritePin(PURGE_GPIO_Port, PURGE_Pin, GPIO_PIN_SET);
        PurgeValveStatus = OPEN;
    }
    else if (status == CLOSED)
    {
        HAL_GPIO_WritePin(PURGE_GPIO_Port, PURGE_Pin, GPIO_PIN_RESET);
        PurgeValveStatus = CLOSED;
    }
}
