#include "can.h"
#include "main.h"
#include "context.h"

/*
o config receberá a mensagem do BMR, mas, no momento,
não está configurado para isso, terremos que pedir o
ID e o slice do DATA para pegar a velocidade
*/

extern FDCAN_HandleTypeDef hfdcan1;
extern FDCAN_TxHeaderTypeDef CHECK;
extern FDCAN_TxHeaderTypeDef ERROR_CAN;
extern FDCAN_FilterTypeDef sFilterConfig;

void config(){
  sFilterConfig.IdType = FDCAN_EXTENDED_ID; // Assumindo ID Extendido
  sFilterConfig.FilterIndex = 0;
  sFilterConfig.FilterType = FDCAN_FILTER_MASK;
  sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
  sFilterConfig.FilterID1 = 0x0000;
  sFilterConfig.FilterID2 = 0x0000;

  HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig);
}

/*
o protocolo vai ser dividido em duas partes:

um header vai ser criado para o envio de informações da fccm,
e o outro vai ser criado para apenas as mensagens de erro
*/

//info da fccm
void declare_can_CHECK(){
  CHECK.Identifier = ID_CHECK;
  CHECK.IdType = FDCAN_EXTENDED_ID;
  CHECK.TxFrameType = FDCAN_DATA_FRAME;
  CHECK.DataLength = FDCAN_DLC_BYTES_8;
  CHECK.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
  CHECK.BitRateSwitch = FDCAN_BRS_OFF;
  CHECK.FDFormat = FDCAN_CLASSIC_CAN; // Mantém modo CAN normal no barramento FDCAN
  CHECK.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
  CHECK.MessageMarker = 0;
}

// declaração do erro
void declare_can_ERROR(){
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

void Slice_DATA(void){
	uint16_t Fccurrent = (uint16_t)(Current);
	uint16_t Fctemp = (uint16_t)(Temp);
	uint16_t Fcvoltage = (uint16_t)(Voltage);
	uint8_t Fcpressure = (uint8_t)(Pressure * 100);
	uint8_t H2 = (uint8_t)(H2_consumed);


    DATA[0] = (uint8_t)((Fccurrent >> 8) & 0xFF);
    DATA[1] = (uint8_t)((Fccurrent) & 0xFF);
    DATA[2] = (uint8_t)((Fctemp >> 8)& 0xFF);
    DATA[3] = (uint8_t)((Fctemp) & 0xFF);
    DATA[4] = (uint8_t)((Fcvoltage >> 8) & 0xFF);
    DATA[5] = (uint8_t)((Fcvoltage) & 0xFF);
    DATA[6] = (uint8_t)((Fcpressure) & 0xFF);
    DATA[7] = (uint8_t)((H2) & 0xFF);
}

void SEND_CAN_Message(void) {
  Slice_DATA();
  HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &CHECK, DATA);
}
