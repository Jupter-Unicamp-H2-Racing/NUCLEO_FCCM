#include "state.h"
#include "contactor.h"
#include "valve.h"
#include "auto_control.h"
#include "uart.h"
#include "button.h"
#include "main.h"

extern int Entry;
extern uint32_t Time;
extern uint8_t ForceSupplyActive;
extern uint8_t ForcePurgeActive;
extern uint32_t ForceSupplyOpenTime;
extern uint32_t ForcePurgeOpenTime;


int StartECU(){
    if (HAL_GPIO_ReadPin(ECM_GPIO_Port, ECM_Pin) == GPIO_PIN_RESET) {
    	return 1;
    }
    return 0;
}

void ForceSupply(void)
{

    if (ForceSupplyActive &&
        (Time - ForceSupplyOpenTime >= 500)) {
    	SupplyValve_CMD(CLOSED);
    	ForceSupplyActive = 0;
    	println("SUPPLY SUPPLY SUPPLY SUPPLY SUPPLY SUPPLY SUPPLY SUPPLY SUPPLY");
    }
}

void ForcePurge(void)
{

    if (ForcePurgeActive &&
        (Time - ForcePurgeOpenTime >= 500)) {
        PurgeValve_CMD(CLOSED);
        ForcePurgeActive = 0;
        println("PURGE PURGE PURGE PURGE PURGE PURGE PURGE PURGE PURGE ");
    }
}

void Air_Starve(){
    Fan_CMD(0);
    SupplyValve_CMD(OPEN);
    PurgeValve_CMD(CLOSED);
    HAL_Delay(10000);
}
