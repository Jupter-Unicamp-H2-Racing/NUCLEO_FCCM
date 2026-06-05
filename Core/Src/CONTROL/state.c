#include "state.h"
#include "contactor.h"
#include "valve.h"
#include "auto_control.h"
#include "uart.h"

extern int Entry;

// Variáveis para controle de delay interno dos estados
int first_delay_auth = 1;
int second_delay_auth = 0;
uint32_t first_delay_start = 0;
uint32_t second_delay_start = 0;
uint32_t startuppurge_start_time;

void FC_StandBy_State(){
    if(Entry == 1){
        Fan_CMD(0);
        SupplyValve_CMD(CLOSED);
        PurgeValve_CMD(CLOSED);
        MainContactor_CMD(OPEN);
        ResistorContactor_CMD(OPEN);
        Entry = 0;
    }
}

void FC_StartUp_FanSpoolUp_State(){
    if(Entry == 1){
        Fan_CMD(FAN_MAX_CMD);
        SupplyValve_CMD(CLOSED);
        PurgeValve_CMD(CLOSED);
        MainContactor_CMD(OPEN);
        ResistorContactor_CMD(OPEN);

        fanspoolup_start_time = Time;
        Entry = 0;
    }else {
        if(Time - fanspoolup_start_time >= FAN_SPOOLUP_DURATION){
            FcActualState = FC_STARTUP_STARTUPPURGE;
            Entry=1;
        }
    }
}

void FC_StartUp_StartUpPurge_State(){
    if(Entry == 1){
        Fan_CMD(FAN_MAX_CMD);
        SupplyValve_CMD(CLOSED);
        PurgeValve_CMD(OPEN); // conferir
        MainContactor_CMD(OPEN);
        ResistorContactor_CMD(OPEN);

        startuppurge_start_time = Time;
        Entry = 0;
    }else {
    	if(Time - startuppurge_start_time >= FAN_SU_PURGE_DURATION){
    		FcActualState = FC_WARMUP;
    		Entry=1;
    	}
    }
}

void FC_WarmUp(){
    if(Entry == 1){
        Fan_CMD(FAN_MIN_CMD);
        SupplyValve_CMD(OPEN);
        PurgeValve_CMD(CLOSED);
        MainContactor_CMD(OPEN);
        ResistorContactor_CMD(CLOSED);

        warmup_start_time = Time;
        Entry = 0;
    }else {
        if(Time - warmup_start_time >= WARMUP_DURATION){
            FcActualState = FC_STARTUP_END;
            Entry=1;
        }
    }
}

void FC_StartUp_End_State(){
    if(Entry == 1){
        Fan_CMD(FAN_MIN_CMD);
        SupplyValve_CMD(OPEN);
        PurgeValve_CMD(CLOSED);
        MainContactor_CMD(OPEN);
        ResistorContactor_CMD(CLOSED); 
        Entry = 0;
    }else {
        FcActualState = FC_RUN;
        Entry=1;
    }
}

void FC_Run_State(){
    if(Entry == 1){
        Automatic_Fan_Control();
        Automatic_Purge_Control();
        SupplyValve_CMD(OPEN);
        MainContactor_CMD(CLOSED);
        ResistorContactor_CMD(OPEN);
        Entry = 0;
    }else {
        Automatic_Fan_Control();
        Automatic_Purge_Control();
        ResistorContactor_CMD(OPEN);
    }
}

void FC_Shutdown_State(){
    if(Entry == 1){
        Fan_CMD(FAN_MAX_CMD);
        SupplyValve_CMD(CLOSED);
        PurgeValve_CMD(CLOSED);
        MainContactor_CMD(OPEN);
        ResistorContactor_CMD(OPEN);

        shutdown_start_time = Time;
        Entry = 0;
    }else {
        if(Time - shutdown_start_time > SHUTDOWN_DURATION){
            FcActualState = FC_STANDBY;
            Entry=1;
        }
    }
}

void FC_Alarm_State(){
    if(Entry == 1){
        Fan_CMD(0);
        SupplyValve_CMD(CLOSED);
        PurgeValve_CMD(CLOSED);
        ResistorContactor_CMD(OPEN);
        MainContactor_CMD(OPEN); // Garantir que desliga
        Entry = 0;
    }
    // Loop infinito ou aguarda reset manual
}

int round_p(float value){
    int under = (int) value;
    int upper = under+1;
    float r_under = value - under;
    float r_upper = upper - value;

    if(r_under < r_upper){
        return (int) value;
    } else{
        return (int) value + 1;
    }
}

int StartECU(){
    if (HAL_GPIO_ReadPin(ECM_GPIO_Port, ECM_Pin) == GPIO_PIN_RESET) {
    	return 1;
    }
    return 0;
}

void ForceSupply(){
    if (HAL_GPIO_ReadPin(FORCE_SUPPLY_GPIO_Port, FORCE_SUPPLY_Pin) == GPIO_PIN_SET) {
    	SupplyValve_CMD(OPEN);
    	HAL_Delay(500);
    	println("supply");
    	SupplyValve_CMD(CLOSED);
    }
}

void ForcePurge(){
    if (HAL_GPIO_ReadPin(FORCE_PURGE_GPIO_Port, FORCE_PURGE_Pin) == GPIO_PIN_SET) {
    	PurgeValve_CMD(OPEN);
    	HAL_Delay(500);
    	println("purga");
    	PurgeValve_CMD(CLOSED);
    }
}

void Air_Starve(){
    Fan_CMD(0);
    SupplyValve_CMD(OPEN);
    PurgeValve_CMD(CLOSED);
    HAL_Delay(10000);
}
