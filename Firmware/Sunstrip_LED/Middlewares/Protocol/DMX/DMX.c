/**
  ******************************************************************************
  * @file           : DMX.c
  * @brief          : Protocol DMX manager receiver
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "DMX.h"

/* Types Definitions ---------------------------------------------------------*/
typedef enum {DMX_NO_ERROR,DMX_ERROR_BUFF
}DMX_Error_Code;

/* Private variables ---------------------------------------------------------*/
uint8_t dmx_rx_buff[1];			//Current byte reception
uint8_t dmx_buff1[DMX_SIZE_CHANNEL];			//Buffer Reception 1
uint8_t dmx_buff2[DMX_SIZE_CHANNEL];			//Buffer Reception 2
uint8_t dmx_buff_valid[DMX_SIZE_CHANNEL];		//Buffer Reception valid

uint16_t dmx_cptByte;
uint16_t dmx_cptAddress;
uint8_t dmx_ref_buffer;

uint8_t dmx_address_begin;
uint8_t dmx_address_end;

UART_HandleTypeDef * dmx_ref_uart;

DMX_Error_Code dmx_Last_Error;

uint32_t dmx_LastTick;

/* Private function -----------------------------------------------*/

/* Public function -----------------------------------------------*/

void Protocol_DMX_init(uint16_t address,UART_HandleTypeDef *ref_uart)
{
	uint8_t i;

	//init variables
	dmx_Last_Error=DMX_NO_ERROR;
	dmx_rx_buff[0]=0;
	dmx_cptAddress=0;
	dmx_cptByte=0;
	dmx_ref_buffer=1;
	dmx_address_begin = address;
	dmx_address_end = address + DMX_SIZE_CHANNEL - 1;
	for (i=0; i<DMX_SIZE_CHANNEL; i++)
	{
		dmx_buff1[i]=0;
		dmx_buff2[i]=0;
		dmx_buff_valid[i]=0;
	}

	dmx_LastTick = HAL_GetTick();

	/* Receive one byte in interrupt mode */
	dmx_ref_uart = ref_uart;
	HAL_UART_Receive_IT(dmx_ref_uart, dmx_rx_buff, 1);

}


void Protocol_DMX_UartCallback(UART_HandleTypeDef *huart)
{
	uint32_t err_code;
	err_code = huart->ErrorCode;
	if (huart->Instance == dmx_ref_uart->Instance)
	{
		/* Receive one byte in interrupt mode */
		HAL_UART_Receive_IT(dmx_ref_uart, dmx_rx_buff, 1);

		//Load buffer
		if(dmx_cptAddress>=dmx_address_begin && dmx_cptAddress<=dmx_address_end)
		{
#ifdef DMX_DOUBLE_READ_CHECK
			if (dmx_ref_buffer==1)
				dmx_buff1[dmx_cptByte]=dmx_rx_buff[0];
			else if (dmx_ref_buffer==2)
				dmx_buff2[dmx_cptByte]=dmx_rx_buff[0];
			else
				dmx_Last_Error=DMX_ERROR_BUFF;

			if(dmx_buff1[dmx_cptByte] == dmx_buff2[dmx_cptByte])
				dmx_buff_valid[dmx_cptByte] = dmx_buff1[dmx_cptByte];
#else
			dmx_buff_valid[dmx_cptByte]=dmx_rx_buff[0];
#endif

			dmx_cptByte++;
		}
		dmx_cptAddress++;

		//New frame detection
		if(err_code==HAL_UART_ERROR_FE)// && rx_buff[0]==0)
		{
			dmx_LastTick = HAL_GetTick();

			dmx_cptAddress=0;
			dmx_cptByte=0;

			if (dmx_ref_buffer==1)
				dmx_ref_buffer=2;
			else if (dmx_ref_buffer==2)
				dmx_ref_buffer=1;
			else
				dmx_Last_Error=DMX_ERROR_BUFF;
		}
	}
}


uint8_t Protocol_DMX_GetValue (uint8_t channel)
{
	if(channel==0 || channel>DMX_SIZE_CHANNEL)
		return 0;
	else
		return dmx_buff_valid[channel-1];
}


uint32_t Protocol_DMX_GetLastTickFrame (void)
{
	return dmx_LastTick;
}

