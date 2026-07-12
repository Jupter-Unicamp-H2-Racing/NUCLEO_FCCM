#include "can.h"
#include "main.h"
#include "context.h"
#include "uart.h"

/*
 * O config() receberá a mensagem do BMR, mas no momento não está
 * configurado para isso — será preciso pedir o ID e o slice do DATA
 * para pegar a velocidade.
 */

/* --- HANDLES EXTERNOS DE CAN --- */
extern FDCAN_HandleTypeDef   hfdcan2;
extern FDCAN_TxHeaderTypeDef CHECK;
extern FDCAN_TxHeaderTypeDef ERROR_CAN;
extern FDCAN_FilterTypeDef   sFilterConfig;
extern FDCAN_RxHeaderTypeDef RxHeader;
extern uint8_t RxData[8];

/* --- CONFIGURAÇÃO DO FILTRO GLOBAL DO FDCAN2 --- */
void config(void)
{
    HAL_FDCAN_ConfigGlobalFilter(&hfdcan2,
        FDCAN_ACCEPT_IN_RX_FIFO0,   // NonMatchingStd
        FDCAN_ACCEPT_IN_RX_FIFO0,   // NonMatchingExt
        FDCAN_FILTER_REMOTE,
        FDCAN_FILTER_REMOTE);
}

/*
 * O protocolo é dividido em duas partes:
 *   - um header para o envio de informações da FCCM (CHECK)
 *   - outro apenas para as mensagens de erro (ERROR_CAN)
 */

/* --- DECLARAÇÃO DO HEADER DE INFO DA FCCM --- */
void declare_can_CHECK(void)
{
    CHECK.Identifier = ID_CHECK;
    CHECK.IdType = FDCAN_EXTENDED_ID;
    CHECK.TxFrameType = FDCAN_DATA_FRAME;
    CHECK.DataLength = FDCAN_DLC_BYTES_8;
    CHECK.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    CHECK.BitRateSwitch = FDCAN_BRS_OFF;
    CHECK.FDFormat = FDCAN_CLASSIC_CAN;   // mantém modo CAN normal no barramento FDCAN
    CHECK.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    CHECK.MessageMarker = 0;
}

/* --- DECLARAÇÃO DO HEADER DE ERRO --- */
void declare_can_ERROR(void)
{
    ERROR_CAN.Identifier = ID_ERROR;
    ERROR_CAN.IdType = FDCAN_EXTENDED_ID;
    ERROR_CAN.TxFrameType = FDCAN_DATA_FRAME;
    ERROR_CAN.DataLength = FDCAN_DLC_BYTES_8;
    ERROR_CAN.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    ERROR_CAN.BitRateSwitch = FDCAN_BRS_OFF;
    ERROR_CAN.FDFormat = FDCAN_CLASSIC_CAN;
    ERROR_CAN.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    ERROR_CAN.MessageMarker = 0;
}

/* --- MONTAGEM DO PAYLOAD DE TELEMETRIA (DATA[8]) --- */
void Slice_DATA(void)
{
    uint16_t Fccurrent  = (uint16_t)(Current);
    uint16_t Fctemp     = (uint16_t)(Temp);
    uint16_t Fcvoltage  = (uint16_t)(Voltage);
    uint8_t  Fcpressure = (uint8_t)(Pressure * 100);
    uint8_t  H2         = (uint8_t)(H2_consumed);

    DATA[0] = (uint8_t)((Fccurrent) & 0xFF);
    DATA[1] = (uint8_t)((Fccurrent >> 8) & 0xFF);
    DATA[2] = (uint8_t)((Fctemp) & 0xFF);
    DATA[3] = (uint8_t)((Fctemp >> 8) & 0xFF);
    DATA[4] = (uint8_t)((Fcvoltage) & 0xFF);
    DATA[5] = (uint8_t)((Fcvoltage >> 8) & 0xFF);
    DATA[6] = (uint8_t)((Fcpressure) & 0xFF);
    DATA[7] = (uint8_t)((H2) & 0xFF);
}
