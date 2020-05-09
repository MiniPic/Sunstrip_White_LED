/**
  ******************************************************************************
  * @file           : DMX.h
  * @brief          : Protocol DMX manager
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */

#ifndef __PROTOCOL_DMX__H__
#define __PROTOCOL_DMX__H__

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Definitions ------------------------------------------------------------------*/
#define DMX_SIZE_CHANNEL		1			//Number of device DMX channels
//#define DMX_DOUBLE_READ_CHECK				//comment if not used

/* Private Definitions ------------------------------------------------------------------*/


/* Public function prototypes -----------------------------------------------*/

/**
  * @brief  add this Callback in UART interrupt
  * @param address: DMX address of device
  */
void Protocol_DMX_init(uint16_t address,UART_HandleTypeDef *ref_uart);

/**
  * @brief  add this Callback in UART interrupt
  * @param ref_uart: UART to use for DMX input signal
  * @param huart: UART interrupt
  */
void Protocol_DMX_UartCallback(UART_HandleTypeDef *huart);

/**
  * @brief  Read value of a DMX channel
  * @param channel: channel of value to read ( from 1 to max channel)
  * @return DMX channel value
  */
uint8_t Protocol_DMX_GetValue (uint8_t channel);

/**
  * @brief  Return last DMX frame tick time
  * @return last DMX frame tick time
  */
uint32_t Protocol_DMX_GetLastTickFrame (void);

#endif /* __PROTOCOL_DMX__H__ */
