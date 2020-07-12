/**
  ******************************************************************************
  * @file           : GENE_I2C_Master.h
  * @brief          : Generated I2C Master from IO open drain
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */

#ifndef __PROTOCOL_GENE_I2C__H__
#define __PROTOCOL_GENE_I2C__H__

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Definitions ------------------------------------------------------------------*/

/* Private Definitions ------------------------------------------------------------------*/
#define		BASE_TIME_US 		16000000/1000000u		//  =Freq/1 000 000
#define		T_BIT				10						// in us

/* Public function prototypes -----------------------------------------------*/
void GENE_I2C_Init(GPIO_TypeDef * SDA_Port, uint16_t SDA_Pin, GPIO_TypeDef * SCL_Port, uint16_t SCL_Pin);
void GENE_I2C_Master_Transmit(uint8_t Address, uint8_t data[], uint16_t count);

#endif
