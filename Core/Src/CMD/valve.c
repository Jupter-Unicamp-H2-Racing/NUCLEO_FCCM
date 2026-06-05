#include "valve.h"
#include "main.h" // Necessário para HAL_GPIO e TIM1

extern int purgas;

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
