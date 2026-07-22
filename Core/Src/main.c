#include "main.h"
#include "context.h"
#include "state.h"
#include "uart.h"
#include "valve.h"
#include "contactor.h"
#include "can.h"
#include "print.h"
#include "button.h"
#include "alarms.h"
#include "auto_control.h"

 /* --- HANDLES DE PERIFÉRICOS (ADC, DMA, CAN, TIMERS, UART) --- */
ADC_HandleTypeDef  hadc1;
ADC_HandleTypeDef  hadc2;

DMA_NodeTypeDef   Node_GPDMA1_Channel0;
DMA_QListTypeDef  List_GPDMA1_Channel0;
DMA_HandleTypeDef handle_GPDMA1_Channel0;

DMA_NodeTypeDef   Node_GPDMA2_Channel0;
DMA_QListTypeDef  List_GPDMA2_Channel0;
DMA_HandleTypeDef handle_GPDMA2_Channel0;

FDCAN_HandleTypeDef     hfdcan2;
FDCAN_TxHeaderTypeDef   CHECK;
FDCAN_TxHeaderTypeDef   ERROR_CAN;
FDCAN_TxHeaderTypeDef   BMS_CAN;
FDCAN_FilterTypeDef     sFilterConfig;
FDCAN_RxHeaderTypeDef   RxHeader;

UART_HandleTypeDef huart2;
UART_HandleTypeDef huart6;

/* --- TIMERS --- */
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim12;


/* --- BUFFERS DE DADOS --- */
uint16_t ADC_VC[100] = {0};   // buffer DMA: tensão/corrente (ADC1)
uint16_t ADC_TP[100] = {0};   // buffer DMA: temperatura/pressão (ADC2)

uint32_t MAILBOX;
uint8_t  DATA[8]    = {0};
uint8_t  ERRO_C[8]  = {0};
uint8_t  RxData[8] = {0};
uint8_t BMS[8] = {0};

/* --- ESTADO DO SISTEMA (válvulas, contatores, máquina de estados) --- */
Valve_Status PurgeValveStatus       = CLOSED;
Valve_Status SupplyValveStatus      = CLOSED;
Valve_Status ResistorContactorStatus = OPEN;
Valve_Status MainContactorStatus     = OPEN;

Fc_State FcActualState = FC_STANDBY;

/* --- VARIÁVEIS DE MEDIÇÃO / SENSORES --- */
float Voltage  = 0;
float Pressure = 0;
int   Temp     = 0;
int   Current  = 0;
int   Fan_power = 0;

float PreviousFanCmd = 0;

/* --- VARIÁVEIS DE CÁLCULO (consumo de H2, corrente) --- */
uint32_t last_H2_calc = 0;
float    H2_consumed  = 0;
float    H2_reacted   = 0;
float    delta_t      = 0;
float    amp_segundo  = 0;
int      purgas       = 1;

/* --- CONTROLE DE TEMPO / TEMPORIZAÇÕES --- */
uint32_t Time = 0;

uint32_t fanspoolup_start_time = 0;
uint32_t warmup_start_time     = 0;
uint32_t shutdown_start_time   = 0;

uint32_t PurgeLastCallTime = 0;
uint32_t PurgeStartTime    = 0;
uint32_t FanLastCallTime   = 0;

float LastITerm     = 0;
int   LastTempDiff  = 0;

/* --- FORÇAR PURGA / SUPRIMENTO (acionamento manual via botão) --- */
uint8_t  ForceSupplyActive   = 0;
uint8_t  ForcePurgeActive    = 0;
uint32_t ForceSupplyOpenTime = 0;
uint32_t ForcePurgeOpenTime  = 0;

/* --- CONTROLE DE FLUXO DA MÁQUINA DE ESTADOS --- */
int Entry         = 1;
int Start         = 0;
int Delay_S       = 0;
int Delay_P       = 0;
int Delay_Restart = 1;

/* --- FLAGS DE TEMPORIZAÇÃO (setadas nas IRQs dos timers) --- */
int flag_5ms   = 0;
int flag_25ms  = 0;
int flag_150ms = 0;

/* --- PROTÓTIPOS DE FUNÇÕES --- */
void SystemClock_Config(void);
static void MPU_Config(void);

static void MX_GPIO_Init(void);
static void MX_GPDMA1_Init(void);
static void MX_GPDMA2_Init(void);
static void MX_ICACHE_Init(void);
static void MX_ADC1_Init(void);
static void MX_ADC2_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART6_UART_Init(void);
static void MX_FDCAN2_Init(void);

static void MX_TIM1_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
static void MX_TIM6_Init(void);
static void MX_TIM12_Init(void);

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc);
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc);
void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin);
void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin);
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);

void Receive_CAN_Message(void);

int main(void)
{
    /* --- Inicialização de baixo nível --- */
    MPU_Config();
    HAL_Init();
    SystemClock_Config();

    /* --- Inicialização de periféricos --- */
    MX_GPIO_Init();
    MX_GPDMA1_Init();
    MX_GPDMA2_Init();
    MX_ICACHE_Init();
    MX_ADC1_Init();
    MX_ADC2_Init();
    MX_USART2_UART_Init();
    MX_USART6_UART_Init();
    MX_FDCAN2_Init();

    /* --- Inicialização dos timers (ordem crescente) --- */
    MX_TIM1_Init();
    MX_TIM2_Init();
    MX_TIM3_Init();
    MX_TIM4_Init();
    MX_TIM6_Init();
    MX_TIM12_Init();

    /* --- Início dos periféricos --- */
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);

    HAL_TIM_Base_Start_IT(&htim3);   // gera flag_5ms
    HAL_TIM_Base_Start_IT(&htim4);   // gera flag_25ms
    HAL_TIM_Base_Start_IT(&htim12);  // gera flag_150ms

    HAL_TIM_Base_Start(&htim6);

    // Força o TIM2 a começar defasado em meio período (180°)
    __HAL_TIM_SET_COUNTER(&htim2, htim2.Init.Period / 2);
    HAL_TIM_Base_Start(&htim2);

    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)&ADC_VC, 100);
    HAL_ADC_Start_DMA(&hadc2, (uint32_t *)&ADC_TP, 100);

    /* --- Inicialização de CAN --- */
    config();
    declare_can_CHECK();
    declare_can_ERROR();
    declare_can_BMS();
    HAL_FDCAN_Start(&hfdcan2);

    println("inicializado com sucesso");

    /* --- Loop principal --- */
    while (1)
    {
        if (flag_5ms)
        {
        	Time = HAL_GetTick();
            Receive_CAN_Message();
            Run_State_Machine();
            Check_Alarms();
            Run_State_Machine();
            flag_5ms = 0;
        }

        if (flag_25ms)
        {
            ForcePurge();
            ForceSupply();
            flag_25ms = 0;
        }

        if (flag_150ms)
        {
            SEND_CAN_Message();
            print_sensors(1, 1);
            flag_150ms = 0;
        }
    }
}

/* --- RECEPÇÃO CAN --- */
void Receive_CAN_Message(void)
{
    if (HAL_FDCAN_GetRxFifoFillLevel(&hfdcan2, FDCAN_RX_FIFO0) > 0)
    {
        if (HAL_FDCAN_GetRxMessage(&hfdcan2, FDCAN_RX_FIFO0, &RxHeader, RxData) == HAL_OK)
        {
            // Códigos de alarme vindos do CAN ID 0x0AF510B2
            if (RxHeader.Identifier == 0x0AF510B2 &&
                (RxData[0] == 1 ||
                 RxData[0] == 2 ||
                 RxData[0] == 10))
            {
                FcActualState = FC_ALARM;
            }

            // Códigos de alarme vindos do CAN ID 0x0AF510B4
            if (RxHeader.Identifier == 0x0AF510B4 &&
                (RxData[7] == 1  ||
                 RxData[7] == 2  ||
                 RxData[7] == 40 ||
                 RxData[7] == 80 ||
                 RxData[6] == 1))
            {
                FcActualState = FC_ALARM;
            }
        }
    }
}

/* --- CALLBACKS DE ADC (DMA half-complete / complete) --- */
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1)
    {
        float voltage_acc = 0;
        float current_acc = 0;

        for (int i = 0; i < 25; i++)
        {
            voltage_acc += ADC_VC[2 * i];
            current_acc += ADC_VC[2 * i + 1];
        }

        voltage_acc /= 20.0f;
        voltage_acc /= 4095.0f;
        voltage_acc *= 100.0f;

        current_acc /= 20.0f;
        current_acc /= 4095.0f;
        current_acc *= 100.0f;

        Voltage = voltage_acc;
        Current = current_acc;
    }

    if (hadc->Instance == ADC2)
    {
        float pressure_acc = 0;
        float temp_acc = 0;

        for (int i = 0; i < 25; i++)
        {
            pressure_acc += ADC_TP[2 * i];
            temp_acc     += ADC_TP[2 * i + 1];
        }

        pressure_acc /= 20.0f;
        pressure_acc /= 4095.0f;
        Pressure = pressure_acc;

        temp_acc /= 20.0f;
        temp_acc /= 4095.0f;
        temp_acc *= 100.0f;
        Temp = temp_acc;
    }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1)
    {
        float voltage_acc = 0;
        float current_acc = 0;

        for (int i = 25; i < 50; i++)
        {
            voltage_acc += ADC_VC[2 * i];
            current_acc += ADC_VC[2 * i + 1];
        }

        voltage_acc /= 20.0f;
        voltage_acc /= 4095.0f;
        voltage_acc *= 100.0f;

        current_acc /= 20.0f;
        current_acc /= 4095.0f;
        current_acc *= 100.0f;

        Voltage = voltage_acc;
        Current = current_acc;
    }

    if (hadc->Instance == ADC2)
    {
        float pressure_acc = 0;
        float temp_acc = 0;

        for (int i = 25; i < 50; i++)
        {
            pressure_acc += ADC_TP[2 * i];
            temp_acc     += ADC_TP[2 * i + 1];
        }

        pressure_acc /= 20.0f;
        pressure_acc /= 4095.0f;
        Pressure = pressure_acc;

        temp_acc /= 20.0f;
        temp_acc /= 4095.0f;
        temp_acc *= 100.0f;
        Temp = temp_acc;
    }
}

/* --- CALLBACKS DE GPIO / EXTI (botões: ECU, force purge, force supply) --- */
void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin)
{
    // Botão ECU: dispara partida do sistema (startup)
    if (GPIO_Pin == ECM_Pin && Delay_Restart)
    {
        if (!Start && FcActualState != FC_ALARM)
        {
            FcActualState = FC_STARTUP_FANSPOOLUP;
            Start = 1;
            Entry = 1;
        }
    }

    // Botão de purga forçada (com debounce de 2000 ms)
    if (GPIO_Pin == FORCE_PURGE_Pin && (2000 <= Time - Delay_P || Delay_P == 0))
    {
        if (!ForcePurgeActive)   // ignora retriggers enquanto já está ativo
        {
            PurgeValve_CMD(OPEN);
            ForcePurgeActive = 1;
            ForcePurgeOpenTime = Time;
            Delay_P = Time;
        }
    }

    // Botão de suprimento forçado (com debounce de 2000 ms)
    if (GPIO_Pin == FORCE_SUPPLY_Pin && (2000 <= Time - Delay_S || Delay_P == 0))
    {
        if (!ForceSupplyActive)   // mesma correção pro supply
        {
            SupplyValve_CMD(OPEN);
            ForceSupplyActive = 1;
            ForceSupplyOpenTime = Time;
            Delay_S = Time;
        }
    }
}

void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
    // Botão ECU solto: dispara shutdown do sistema
    if (GPIO_Pin == ECM_Pin)
    {
        if (StartECU() && Start && FcActualState != FC_ALARM)
        {
            FcActualState = FC_SHUTDOWN;
            Start = 0;
            Delay_Restart = 0;
        }
    }
}

/* --- CALLBACK DE TIMERS (gera as flags de temporização) --- */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM3)
    {
        flag_5ms = 1;
    }
    if (htim->Instance == TIM4)
    {
        flag_25ms = 1;
    }
    if (htim->Instance == TIM12)
    {
        flag_150ms = 1;
    }
}

/* =========================================================================
 *  CONFIGURAÇÃO DE CLOCK
 * ========================================================================= */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /* Configura a tensão do regulador interno principal */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
    while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

    /* Inicializa os osciladores da RCC */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV2;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLL1_SOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 3;
    RCC_OscInitStruct.PLL.PLLN = 60;
    RCC_OscInitStruct.PLL.PLLP = 6;
    RCC_OscInitStruct.PLL.PLLQ = 6;
    RCC_OscInitStruct.PLL.PLLR = 2;
    RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1_VCIRANGE_3;
    RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1_VCORANGE_WIDE;
    RCC_OscInitStruct.PLL.PLLFRACN = 0;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /* Inicializa os clocks de CPU, AHB e APB */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                 | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2
                                 | RCC_CLOCKTYPE_PCLK3;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
    {
        Error_Handler();
    }

    /* Configura o delay de programação da flash */
    __HAL_FLASH_SET_PROGRAM_DELAY(FLASH_PROGRAMMING_DELAY_0);
}

/* =========================================================================
 *  ADC1 - Tensão / Corrente
 * ========================================================================= */
static void MX_ADC1_Init(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    /* Configuração comum */
    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    hadc1.Init.LowPowerAutoWait = DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.NbrOfConversion = 2;
    hadc1.Init.DiscontinuousConvMode = ENABLE;
    hadc1.Init.NbrOfDiscConversion = 1;   // 1 canal por trigger
    hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIG_T6_TRGO;   // ver RM0481, tabela de triggers do ADC1
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
    hadc1.Init.DMAContinuousRequests = ENABLE;
    hadc1.Init.SamplingMode = ADC_SAMPLING_MODE_BULB;
    hadc1.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
    hadc1.Init.OversamplingMode = ENABLE;
    hadc1.Init.Oversampling.Ratio = ADC_OVERSAMPLING_RATIO_16;   // soma 16 amostras
    hadc1.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_4;
    if (HAL_ADC_Init(&hadc1) != HAL_OK)
    {
        Error_Handler();
    }

    /* Canal regular 1 */
    sConfig.Channel = ADC_CHANNEL_0;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_640CYCLES_5;
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }

    /* Canal regular 2 */
    sConfig.Channel = ADC_CHANNEL_1;
    sConfig.Rank = ADC_REGULAR_RANK_2;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }
}

/* =========================================================================
 *  ADC2 - Temperatura / Pressão
 * ========================================================================= */
static void MX_ADC2_Init(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    /* Configuração comum */
    hadc2.Instance = ADC2;
    hadc2.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
    hadc2.Init.Resolution = ADC_RESOLUTION_12B;
    hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc2.Init.ScanConvMode = ADC_SCAN_ENABLE;
    hadc2.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    hadc2.Init.LowPowerAutoWait = DISABLE;
    hadc2.Init.ContinuousConvMode = DISABLE;
    hadc2.Init.NbrOfConversion = 2;
    hadc2.Init.DiscontinuousConvMode = DISABLE;
    hadc2.Init.ExternalTrigConv = ADC_EXTERNALTRIG_T2_TRGO;
    hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
    hadc2.Init.DMAContinuousRequests = ENABLE;
    hadc2.Init.SamplingMode = ADC_SAMPLING_MODE_NORMAL;
    hadc2.Init.Overrun = ADC_OVR_DATA_PRESERVED;
    hadc2.Init.OversamplingMode = DISABLE;
    if (HAL_ADC_Init(&hadc2) != HAL_OK)
    {
        Error_Handler();
    }

    /* Canal regular 1 */
    sConfig.Channel = ADC_CHANNEL_9;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_640CYCLES_5;
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;
    if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }

    /* Canal regular 2 */
    sConfig.Channel = ADC_CHANNEL_13;
    sConfig.Rank = ADC_REGULAR_RANK_2;
    if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }
}

/* =========================================================================
 *  FDCAN2
 * ========================================================================= */
static void MX_FDCAN2_Init(void)
{
    hfdcan2.Instance = FDCAN2;
    hfdcan2.Init.ClockDivider = FDCAN_CLOCK_DIV1;
    hfdcan2.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
    hfdcan2.Init.Mode = FDCAN_MODE_NORMAL;
    hfdcan2.Init.AutoRetransmission = DISABLE;
    hfdcan2.Init.TransmitPause = DISABLE;
    hfdcan2.Init.ProtocolException = DISABLE;
    hfdcan2.Init.NominalPrescaler = 8;
    hfdcan2.Init.NominalSyncJumpWidth = 1;
    hfdcan2.Init.NominalTimeSeg1 = 15;
    hfdcan2.Init.NominalTimeSeg2 = 4;
    hfdcan2.Init.DataPrescaler = 1;
    hfdcan2.Init.DataSyncJumpWidth = 1;
    hfdcan2.Init.DataTimeSeg1 = 1;
    hfdcan2.Init.DataTimeSeg2 = 1;
    hfdcan2.Init.StdFiltersNbr = 0;
    hfdcan2.Init.ExtFiltersNbr = 0;
    hfdcan2.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
    if (HAL_FDCAN_Init(&hfdcan2) != HAL_OK)
    {
        Error_Handler();
    }
}

/* =========================================================================
 *  GPDMA1 / GPDMA2
 * ========================================================================= */
static void MX_GPDMA1_Init(void)
{
    __HAL_RCC_GPDMA1_CLK_ENABLE();

    HAL_NVIC_SetPriority(GPDMA1_Channel0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(GPDMA1_Channel0_IRQn);
}

static void MX_GPDMA2_Init(void)
{
    __HAL_RCC_GPDMA2_CLK_ENABLE();

    HAL_NVIC_SetPriority(GPDMA2_Channel0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(GPDMA2_Channel0_IRQn);
}

/* =========================================================================
 *  ICACHE
 * ========================================================================= */
static void MX_ICACHE_Init(void)
{
    /* Habilita o cache de instrução em modo 1-way (mapeamento direto) */
    if (HAL_ICACHE_ConfigAssociativityMode(ICACHE_1WAY) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_ICACHE_Enable() != HAL_OK)
    {
        Error_Handler();
    }
}

/* =========================================================================
 *  TIM1 - PWM
 * ========================================================================= */
static void MX_TIM1_Init(void)
{
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};
    TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

    htim1.Instance = TIM1;
    htim1.Init.Prescaler = 320;
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Period = 100;
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
    {
        Error_Handler();
    }

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }

    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
    {
        Error_Handler();
    }

    sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
    sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
    sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
    sBreakDeadTimeConfig.DeadTime = 0;
    sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
    sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
    sBreakDeadTimeConfig.BreakFilter = 0;
    sBreakDeadTimeConfig.BreakAFMode = TIM_BREAK_AFMODE_INPUT;
    sBreakDeadTimeConfig.Break2State = TIM_BREAK2_DISABLE;
    sBreakDeadTimeConfig.Break2Polarity = TIM_BREAK2POLARITY_HIGH;
    sBreakDeadTimeConfig.Break2Filter = 0;
    sBreakDeadTimeConfig.Break2AFMode = TIM_BREAK_AFMODE_INPUT;
    sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
    if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
    {
        Error_Handler();
    }

    HAL_TIM_MspPostInit(&htim1);
}

/* =========================================================================
 *  TIM2 - Trigger para ADC2 (defasado 180° em relação ao TIM6)
 * ========================================================================= */
static void MX_TIM2_Init(void)
{
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    __HAL_RCC_TIM2_CLK_ENABLE();

    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 31999;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 1;   // mesmo período do TIM6
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
    {
        Error_Handler();
    }

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
}

/* =========================================================================
 *  TIM3
 * ========================================================================= */
static void MX_TIM3_Init(void)
{
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 31999;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 4;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
    {
        Error_Handler();
    }

    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
    {
        Error_Handler();
    }

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
}

/* =========================================================================
 *  TIM4
 * ========================================================================= */
static void MX_TIM4_Init(void)
{
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim4.Instance = TIM4;
    htim4.Init.Prescaler = 31999;
    htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim4.Init.Period = 24;
    htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
    {
        Error_Handler();
    }

    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
    {
        Error_Handler();
    }

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
}

/* =========================================================================
 *  TIM6 - Trigger para ADC1
 * ========================================================================= */
static void MX_TIM6_Init(void)
{
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    __HAL_RCC_TIM6_CLK_ENABLE();

    htim6.Instance = TIM6;
    htim6.Init.Prescaler = 31999;   // ajustar para a taxa de amostragem desejada
    htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim6.Init.Period = 1;         // período menor = amostragem mais rápida
    htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
    {
        Error_Handler();
    }

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;   // TRGO no update event
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
}

/* =========================================================================
 *  TIM12
 * ========================================================================= */
static void MX_TIM12_Init(void)
{
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};

    htim12.Instance = TIM12;
    htim12.Init.Prescaler = 31999;
    htim12.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim12.Init.Period = 149;
    htim12.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim12.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim12) != HAL_OK)
    {
        Error_Handler();
    }

    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim12, &sClockSourceConfig) != HAL_OK)
    {
        Error_Handler();
    }
}

/* =========================================================================
 *  USART2
 * ========================================================================= */
static void MX_USART2_UART_Init(void)
{
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&huart2) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_UARTEx_SetTxFifoThreshold(&huart2, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_UARTEx_SetRxFifoThreshold(&huart2, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_UARTEx_DisableFifoMode(&huart2) != HAL_OK)
    {
        Error_Handler();
    }
}

/* =========================================================================
 *  USART6
 * ========================================================================= */
static void MX_USART6_UART_Init(void)
{
    /* !!! HAL_UART_Init está comentado pois faltam parâmetros de configuração !!! */
    // huart2.Instance = USART6;
    // huart2.Init.BaudRate = 115200;
    // huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    // huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    // huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    // HAL_UART_Init(&huart2);

    /* !!! Configuração de FIFO comentada pelo mesmo motivo !!! */
    // HAL_UARTEx_SetTxFifoThreshold(&huart2, UART_TXFIFO_THRESHOLD_1_8);
    // HAL_UARTEx_SetRxFifoThreshold(&huart2, UART_RXFIFO_THRESHOLD_1_8);
    // HAL_UARTEx_DisableFifoMode(&huart2);
}

/* =========================================================================
 *  GPIO
 * ========================================================================= */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Habilita clock dos ports usados */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* Nível inicial dos pinos de saída */
    HAL_GPIO_WritePin(GPIOC, RESISTOR_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, SUPPLY_Pin | MAIN_Pin | PURGE_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, AL1_Pin | AL2_Pin, GPIO_PIN_SET);

    /* Pinos de saída: AL1_Pin, RESISTOR_Pin, AL2_Pin */
    GPIO_InitStruct.Pin = AL1_Pin | RESISTOR_Pin | AL2_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* Pino de entrada com interrupção (subida e descida): ECM_Pin */
    GPIO_InitStruct.Pin = ECM_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(ECM_GPIO_Port, &GPIO_InitStruct);

    /* Pinos de entrada com interrupção (subida): FORCE_PURGE_Pin, FORCE_SUPPLY_Pin */
    GPIO_InitStruct.Pin = FORCE_PURGE_Pin | FORCE_SUPPLY_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* Pinos de saída: SUPPLY_Pin, MAIN_Pin, PURGE_Pin */
    GPIO_InitStruct.Pin = SUPPLY_Pin | MAIN_Pin | PURGE_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* Configuração das interrupções externas (EXTI) */
    HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);   // ECM_Pin
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);

    HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);   // FORCE_PURGE_Pin
    HAL_NVIC_EnableIRQ(EXTI1_IRQn);

    HAL_NVIC_SetPriority(EXTI2_IRQn, 0, 0);   // FORCE_SUPPLY_Pin
    HAL_NVIC_EnableIRQ(EXTI2_IRQn);
}

/* =========================================================================
 *  HANDLERS DE INTERRUPÇÃO EXTERNA (EXTI)
 * ========================================================================= */
void EXTI0_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(ECM_Pin);
}

void EXTI1_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(FORCE_PURGE_Pin);
}

void EXTI2_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(FORCE_SUPPLY_Pin);
}

/* =========================================================================
 *  CONFIGURAÇÃO DA MPU
 * ========================================================================= */
void MPU_Config(void)
{
    MPU_Region_InitTypeDef MPU_InitStruct = {0};
    MPU_Attributes_InitTypeDef MPU_AttributesInit = {0};

    HAL_MPU_Disable();

    /* Região 0: protege a área de memória especificada */
    MPU_InitStruct.Enable = MPU_REGION_ENABLE;
    MPU_InitStruct.Number = MPU_REGION_NUMBER0;
    MPU_InitStruct.BaseAddress = 0x08FFF000;
    MPU_InitStruct.LimitAddress = 0x08FFFFFF;
    MPU_InitStruct.AttributesIndex = MPU_ATTRIBUTES_NUMBER0;
    MPU_InitStruct.AccessPermission = MPU_REGION_ALL_RO;
    MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
    MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    /* Atributo 0: memória não cacheável */
    MPU_AttributesInit.Number = MPU_ATTRIBUTES_NUMBER0;
    MPU_AttributesInit.Attributes = INNER_OUTER(MPU_NOT_CACHEABLE);
    HAL_MPU_ConfigMemoryAttributes(&MPU_AttributesInit);

    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

/**
 * @brief  Executada em caso de ocorrência de erro.
 * @retval None
 */
void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
        /* Loop infinito de erro */
    }
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reporta o nome do arquivo e o número da linha onde
 *         ocorreu o erro de assert_param.
 * @param  file: ponteiro para o nome do arquivo fonte
 * @param  line: número da linha onde ocorreu o erro
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* Implementação opcional para reportar arquivo/linha do erro */
}
#endif /* USE_FULL_ASSERT */
