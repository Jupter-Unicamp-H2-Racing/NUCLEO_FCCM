#include "valve.h"
#include "main.h" // Necessário para HAL_GPIO e TIM1

extern int purgas;
extern float Pressure;
extern float Voltage;
extern int Temp;
extern int Current;
extern int Entry;
extern uint32_t Time;

void H2_reaction_tracker(void)
{

    H2_reacted = H2_REACTION_PER_SEC_A * Current * PurgeLastCallTime;

    if (H2_consumed + H2_reacted >= H2_TOTAL_MASS) {
        H2_consumed = H2_TOTAL_MASS;
    } else {
        H2_consumed += H2_reacted;
    }
}

void H2_purge_tracker(){
  if (H2_consumed + H2_PURGE_CONSUMPTION >= H2_TOTAL_MASS) {
       H2_consumed = H2_TOTAL_MASS;
  }
  else {
       H2_consumed += H2_PURGE_CONSUMPTION;
  }

}

void SupplyValve_CMD(Valve_Status status){
    if(status == OPEN){
        HAL_GPIO_WritePin(SUPPLY_GPIO_Port, SUPPLY_Pin, GPIO_PIN_SET);
        SupplyValveStatus = OPEN;
    } else if (status == CLOSED){
        HAL_GPIO_WritePin(SUPPLY_GPIO_Port, SUPPLY_Pin, GPIO_PIN_RESET);
        SupplyValveStatus = CLOSED;
    }
}

void PurgeValve_CMD(Valve_Status status){
    if(status == OPEN){
        H2_purge_tracker();
        purgas += 1;
        HAL_GPIO_WritePin(PURGE_GPIO_Port, PURGE_Pin, GPIO_PIN_SET);
        PurgeValveStatus = OPEN;
    } else if (status == CLOSED){
        HAL_GPIO_WritePin(PURGE_GPIO_Port, PURGE_Pin, GPIO_PIN_RESET);
        PurgeValveStatus = CLOSED;
    }
}
