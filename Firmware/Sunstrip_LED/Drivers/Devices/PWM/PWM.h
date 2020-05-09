/**
  ******************************************************************************
  * @file           : PWM.h
  * @brief          : Manage PWM
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */

#ifndef DEVICES_PWM_H_
#define DEVICES_PWM_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Definitions ------------------------------------------------------------------*/

/* Private Definitions ------------------------------------------------------------------*/

/* Public function prototypes -----------------------------------------------*/
void PWM_SetPWM(TIM_HandleTypeDef* timer, uint32_t channel, uint16_t period,uint16_t pulse);
void PWM_SetDuty(TIM_HandleTypeDef* timer, uint32_t channel, uint16_t pulse);

#endif /* DEVICES_PWM_H_ */
