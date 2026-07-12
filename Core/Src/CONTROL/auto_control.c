#include "auto_control.h"
#include "context.h"
#include "state.h"
#include "valve.h"

/* --- VARIÁVEIS LOCAIS --- */
float Topt;              // temperatura ótima calculada (uso interno deste arquivo)
int   PurgeValveCmd  = CLOSED;
int   PurgeActive    = 0;
extern uint8_t ForcePurgeActive;

/* --- CÁLCULO DA TEMPERATURA ÓTIMA --- */
float Calculate_OptTemp(void)
{
    float Topt_calc = (4.0f / 7.3f) * Current + 26.0f;
    return Topt_calc;
}

/* --- CONTROLE AUTOMÁTICO DA FAN (PID) --- */
void Automatic_Fan_Control(void)
{
    Topt = Calculate_OptTemp();

    int TempDiff = Temp - (int)Topt;

    // Se passou muito tempo, reseta as variáveis integrais (anti-windup reset)
    if (Time - FanLastCallTime >= RESET_TIME)
    {
        LastITerm = PreviousFanCmd;
        LastTempDiff = TempDiff;
    }

    // Termo proporcional
    float PTerm = P_CONST * TempDiff;

    // Termo integral
    float dt = ((float)Time - (float)FanLastCallTime) / 1000.0f;
    float ITerm = (I_CONST * TempDiff * dt) + LastITerm;

    // Limitação do termo integral
    if (ITerm > FAN_MAX_CMD)
    {
        ITerm = FAN_MAX_CMD;
    }
    if (ITerm < FAN_MIN_CMD)
    {
        ITerm = FAN_MIN_CMD;
    }

    // Termo derivativo
    float DTerm = 0;
    if (dt > 0)
    {
        DTerm = D_CONST * (TempDiff - LastTempDiff) / dt;
    }

    // Combina os termos
    float UpdatedFanCmd = PTerm + ITerm + DTerm;

    // Limites finais de saída
    if (UpdatedFanCmd > FAN_MAX_CMD)
    {
        UpdatedFanCmd = FAN_MAX_CMD;
    }
    if (UpdatedFanCmd < FAN_MIN_CMD)
    {
        UpdatedFanCmd = FAN_MIN_CMD;
    }

    // Atualiza variáveis estáticas de controle
    FanLastCallTime = Time;
    LastTempDiff = TempDiff;
    LastITerm = ITerm;
    PreviousFanCmd = UpdatedFanCmd;

    // Envia o comando para a fan
    Fan_CMD((int)UpdatedFanCmd);
}

/* --- CONTROLE AUTOMÁTICO DE PURGA (baseado em amp*s) --- */
void Automatic_Purge_Control(void)
{
    if (ForcePurgeActive)
    {
        return;   // force tem prioridade; ele mesmo fecha a válvula quando o tempo vencer
    }

    if (PurgeValveStatus == CLOSED && Current > 0)
    {
        float dt_s = ((float)(Time - PurgeLastCallTime)) / 1000.0f;
        amp_segundo += Current * dt_s;
    }

    PurgeLastCallTime = Time;

    if (!PurgeActive)
    {
        if (amp_segundo >= PURGE_INTERVAL)
        {
            amp_segundo = 0.0f;
            PurgeStartTime = Time;
            PurgeActive = 1;
            PurgeValve_CMD(OPEN);
        }
        else
        {
            PurgeValve_CMD(CLOSED);
        }
    }
    else
    {
        if (Time - PurgeStartTime >= PURGE_DURATION)
        {
            PurgeValve_CMD(CLOSED);
            PurgeActive = 0;
        }
    }
}
