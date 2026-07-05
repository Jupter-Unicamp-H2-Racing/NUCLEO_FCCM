/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h5xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define ECM_Pin GPIO_PIN_0
#define ECM_GPIO_Port GPIOC
#define ECM_EXTI_IRQn EXTI0_IRQn
#define RESISTOR_Pin GPIO_PIN_2
#define RESISTOR_GPIO_Port GPIOC
#define TEMPERATURE_Pin GPIO_PIN_3
#define TEMPERATURE_GPIO_Port GPIOC
#define VOLTAGE_Pin GPIO_PIN_0
#define VOLTAGE_GPIO_Port GPIOA
#define CURRENT_Pin GPIO_PIN_7
#define CURRENT_GPIO_Port GPIOA
#define PRESSURE_Pin GPIO_PIN_0
#define PRESSURE_GPIO_Port GPIOB
#define MAIN_Pin GPIO_PIN_12
#define MAIN_GPIO_Port GPIOB
#define PURGE_Pin GPIO_PIN_13
#define PURGE_GPIO_Port GPIOB
#define PWM_FAN_Pin GPIO_PIN_8
#define PWM_FAN_GPIO_Port GPIOA
#define FORCE_SUPPLY_Pin GPIO_PIN_9
#define FORCE_SUPPLY_GPIO_Port GPIOA
#define FORCE_SUPPLY_EXTI_IRQn EXTI9_IRQn
#define FORCE_PURGE_Pin GPIO_PIN_10
#define FORCE_PURGE_GPIO_Port GPIOA
#define FORCE_PURGE_EXTI_IRQn EXTI10_IRQn
#define SUPPLY_Pin GPIO_PIN_11
#define SUPPLY_GPIO_Port GPIOC

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
