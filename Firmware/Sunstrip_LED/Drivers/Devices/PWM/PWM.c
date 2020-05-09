/**
  ******************************************************************************
  * @file           : PWM.c
  * @brief          : Manage PWM
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "PWM.h"

/* Types Definitions ---------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/


/* Private function -----------------------------------------------*/


/* Public function -----------------------------------------------*/
void PWM_SetPWM(TIM_HandleTypeDef* timer, uint32_t channel, uint16_t period,uint16_t pulse)
{
	 HAL_TIM_PWM_Stop(timer, channel); // stop generation of pwm
	 TIM_OC_InitTypeDef sConfigOC;
	 timer->Init.Period = period; // set the period duration
	 HAL_TIM_PWM_Init(timer); // reinititialise with new period value
	 sConfigOC.OCMode = TIM_OCMODE_PWM1;
	 sConfigOC.Pulse = pulse; // set the pulse duration
	 sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	 sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	 HAL_TIM_PWM_ConfigChannel(timer, &sConfigOC, channel);
	 HAL_TIM_PWM_Start(timer, channel); // start pwm generation
}

void PWM_SetDuty(TIM_HandleTypeDef* timer, uint32_t channel, uint16_t pulse)
{
	 TIM_OC_InitTypeDef sConfigOC;
	 sConfigOC.OCMode = TIM_OCMODE_PWM1;
	 sConfigOC.Pulse = pulse; // set the pulse duration
	 sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	 sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	 __HAL_TIM_SET_COMPARE(timer, channel, pulse);
}
