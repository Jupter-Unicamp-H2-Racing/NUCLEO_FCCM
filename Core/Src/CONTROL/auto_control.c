#include "auto_control.h"
#include "state.h"
#include "valve.h"

float Topt;// Variável global interna deste arquivo (ou mova para context se precisar ver fora)
int PurgeValveCmd = CLOSED;
int PurgeActive = 0;
extern uint8_t ForcePurgeActive;

float Calculate_OptTemp(){
    // Nota: FcCurrent vem do fuel_cell_context.h
    float Topt_calc = (4.0f / 7.3f) * Current + 26.0f;
    return Topt_calc;
}

void Automatic_Fan_Control(){
    // Calculate OptTemp based on FcCurrent
    Topt = Calculate_OptTemp();

    // Calculate Temp Difference
    int TempDiff = Temp - (int)Topt;

    // Se passou muito tempo, reseta as variáveis integrais (Anti-windup reset)
    if (Time - FanLastCallTime >= RESET_TIME){
        LastITerm = PreviousFanCmd;
        LastTempDiff = TempDiff;
    }

    // Termo Proporcional
    float PTerm = P_CONST * TempDiff;

    // Termo Integral
    float dt = ((float)Time - (float)FanLastCallTime) / 1000.0f;
    float ITerm = (I_CONST * TempDiff * dt) + LastITerm;

    //Limitação do ITerm
    if(ITerm > FAN_MAX_CMD){
    	ITerm = FAN_MAX_CMD;
    }
    if(ITerm < FAN_MIN_CMD){
        ITerm = FAN_MIN_CMD;
    }

    // Termo Derivativo
    float DTerm = 0;
    if(dt > 0){
        DTerm = D_CONST * (TempDiff - LastTempDiff) / dt;
    }

    // Combine Terms
    float UpdatedFanCmd = PTerm + ITerm + DTerm;

    // Limites
    if (UpdatedFanCmd > FAN_MAX_CMD){
        UpdatedFanCmd = FAN_MAX_CMD;
    }
    if (UpdatedFanCmd < FAN_MIN_CMD){
        UpdatedFanCmd = FAN_MIN_CMD;
    }

    // Atualiza estáticos
    FanLastCallTime = Time;
    LastTempDiff = TempDiff;
    LastITerm = ITerm;
    PreviousFanCmd = UpdatedFanCmd;

    // Envia sinal
    Fan_CMD((int)UpdatedFanCmd);
}

void Automatic_Purge_Control() {
    if (ForcePurgeActive) {
        return;   // Force tem prioridade; ele mesmo fecha a válvula quando o tempo vencer
    }

    if (PurgeValveStatus == CLOSED && Current > 0) {
        float dt_s = ((float)(Time - PurgeLastCallTime)) / 1000.0f;
        amp_segundo += Current * dt_s;
    }

    PurgeLastCallTime = Time;

    if (!PurgeActive) {
        if (amp_segundo >= PURGE_INTERVAL) {
            amp_segundo = 0.0f;
            PurgeStartTime = Time;
            PurgeActive = 1;
            PurgeValve_CMD(OPEN);
        } else {
            PurgeValve_CMD(CLOSED);
        }
    } else {
        if (Time - PurgeStartTime >= PURGE_DURATION) {
            PurgeValve_CMD(CLOSED);
            PurgeActive = 0;
        }
    }
}
