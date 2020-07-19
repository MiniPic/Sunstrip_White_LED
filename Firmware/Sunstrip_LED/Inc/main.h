/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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
#include "stm32f1xx_hal.h"

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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_V_Pin GPIO_PIN_13
#define LED_V_GPIO_Port GPIOC
#define T1_Pin GPIO_PIN_14
#define T1_GPIO_Port GPIOC
#define RE_DE_Pin GPIO_PIN_15
#define RE_DE_GPIO_Port GPIOC
#define PWM5_Pin GPIO_PIN_0
#define PWM5_GPIO_Port GPIOA
#define PWM6_Pin GPIO_PIN_1
#define PWM6_GPIO_Port GPIOA
#define DMX_OUT_Pin GPIO_PIN_2
#define DMX_OUT_GPIO_Port GPIOA
#define DMX_IN_Pin GPIO_PIN_3
#define DMX_IN_GPIO_Port GPIOA
#define CTN_Pin GPIO_PIN_7
#define CTN_GPIO_Port GPIOA
#define FAN_Pin GPIO_PIN_0
#define FAN_GPIO_Port GPIOB
#define VOLTAGE_Pin GPIO_PIN_1
#define VOLTAGE_GPIO_Port GPIOB
#define PWM7_Pin GPIO_PIN_10
#define PWM7_GPIO_Port GPIOB
#define PWM8_Pin GPIO_PIN_11
#define PWM8_GPIO_Port GPIOB
#define T2_Pin GPIO_PIN_12
#define T2_GPIO_Port GPIOB
#define T3_Pin GPIO_PIN_13
#define T3_GPIO_Port GPIOB
#define PWM4_Pin GPIO_PIN_8
#define PWM4_GPIO_Port GPIOA
#define PWM3_Pin GPIO_PIN_9
#define PWM3_GPIO_Port GPIOA
#define PWM2_Pin GPIO_PIN_10
#define PWM2_GPIO_Port GPIOA
#define PWM1_Pin GPIO_PIN_11
#define PWM1_GPIO_Port GPIOA
#define PWM9_Pin GPIO_PIN_4
#define PWM9_GPIO_Port GPIOB
#define PWM10_Pin GPIO_PIN_5
#define PWM10_GPIO_Port GPIOB
#define USART_TX_DUBUG_Pin GPIO_PIN_6
#define USART_TX_DUBUG_GPIO_Port GPIOB
#define USART_RX_DUBUG_Pin GPIO_PIN_7
#define USART_RX_DUBUG_GPIO_Port GPIOB
#define SCL_IO_Pin GPIO_PIN_8
#define SCL_IO_GPIO_Port GPIOB
#define SDA_IO_Pin GPIO_PIN_9
#define SDA_IO_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
