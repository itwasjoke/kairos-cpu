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
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "RS485.h"
#include "CAN.h"
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
#define AOUT_LED0_Pin GPIO_PIN_5
#define AOUT_LED0_GPIO_Port GPIOE
#define AOUT_LED1_Pin GPIO_PIN_6
#define AOUT_LED1_GPIO_Port GPIOE
#define DIN3_Pin GPIO_PIN_0
#define DIN3_GPIO_Port GPIOA
#define AOUT0_Pin GPIO_PIN_4
#define AOUT0_GPIO_Port GPIOA
#define AOUT1_Pin GPIO_PIN_5
#define AOUT1_GPIO_Port GPIOA
#define DIN2_Pin GPIO_PIN_6
#define DIN2_GPIO_Port GPIOA
#define LED_RS_TX_Pin GPIO_PIN_8
#define LED_RS_TX_GPIO_Port GPIOE
#define LED_RS_RX_Pin GPIO_PIN_9
#define LED_RS_RX_GPIO_Port GPIOE
#define LED_CAN_TX_Pin GPIO_PIN_10
#define LED_CAN_TX_GPIO_Port GPIOE
#define LED_CAN_RX_Pin GPIO_PIN_11
#define LED_CAN_RX_GPIO_Port GPIOE
#define LED1_Pin GPIO_PIN_12
#define LED1_GPIO_Port GPIOE
#define LED2_Pin GPIO_PIN_13
#define LED2_GPIO_Port GPIOE
#define LED3_Pin GPIO_PIN_14
#define LED3_GPIO_Port GPIOE
#define LED4_Pin GPIO_PIN_15
#define LED4_GPIO_Port GPIOE
#define AIN_LED0_Pin GPIO_PIN_6
#define AIN_LED0_GPIO_Port GPIOC
#define AIN_LED1_Pin GPIO_PIN_7
#define AIN_LED1_GPIO_Port GPIOC
#define AIN_LED2_Pin GPIO_PIN_8
#define AIN_LED2_GPIO_Port GPIOC
#define AIN_LED3_Pin GPIO_PIN_9
#define AIN_LED3_GPIO_Port GPIOC
#define DIN0_Pin GPIO_PIN_8
#define DIN0_GPIO_Port GPIOA
#define RS485_DE_Pin GPIO_PIN_12
#define RS485_DE_GPIO_Port GPIOC
#define DOUT0_Pin GPIO_PIN_0
#define DOUT0_GPIO_Port GPIOD
#define DOUT1_Pin GPIO_PIN_1
#define DOUT1_GPIO_Port GPIOD
#define DOUT2_Pin GPIO_PIN_2
#define DOUT2_GPIO_Port GPIOD
#define DOUT3_Pin GPIO_PIN_3
#define DOUT3_GPIO_Port GPIOD

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
