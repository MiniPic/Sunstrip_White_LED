/**
  ******************************************************************************
  * @file           : GENE_I2C_Master.c
  * @brief          : Generated I2C Master from IO open drain
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "GENE_I2C_Master.h"

/* Types Definitions ---------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
GPIO_TypeDef * 	pSDA_Port;
uint16_t 		iSDA_Pin;
GPIO_TypeDef * 	pSCL_Port;
uint16_t 		iSCL_Pin;


/* Private function -----------------------------------------------*/

/* Public function -----------------------------------------------*/
void GENE_I2C_Init(GPIO_TypeDef * SDA_Port, uint16_t SDA_Pin, GPIO_TypeDef * SCL_Port, uint16_t SCL_Pin)
{
	pSDA_Port = SDA_Port;
	pSCL_Port = SCL_Port;
	iSDA_Pin = SDA_Pin;
	iSCL_Pin = SCL_Pin;

	HAL_GPIO_WritePin(pSDA_Port, iSDA_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(pSCL_Port, iSCL_Pin, GPIO_PIN_SET);
}

void GENE_I2C_Master_Transmit(uint8_t Address, uint8_t data[], uint16_t count)
{
	uint8_t i,j,d;

	//START
	HAL_GPIO_WritePin(pSDA_Port, iSDA_Pin, GPIO_PIN_RESET);
	I2C_usDelay(1);
	HAL_GPIO_WritePin(pSCL_Port, iSCL_Pin, GPIO_PIN_RESET);
	I2C_usDelay(1);

	for(i=7; i>0; i--)	//Address b7-1
	{
		HAL_GPIO_WritePin(pSDA_Port, iSDA_Pin, (Address>>i)&0x0001);
		I2C_usDelay(1);
		HAL_GPIO_WritePin(pSCL_Port, iSCL_Pin, GPIO_PIN_SET);
		I2C_usDelay(1);
		HAL_GPIO_WritePin(pSCL_Port, iSCL_Pin, GPIO_PIN_RESET);
		I2C_usDelay(1);
	}

	//Address b0
	HAL_GPIO_WritePin(pSDA_Port, iSDA_Pin, GPIO_PIN_RESET);
	I2C_usDelay(1);
	HAL_GPIO_WritePin(pSCL_Port, iSCL_Pin, GPIO_PIN_SET);
	I2C_usDelay(1);
	HAL_GPIO_WritePin(pSCL_Port, iSCL_Pin, GPIO_PIN_RESET);
	I2C_usDelay(1);

	//ACK
	HAL_GPIO_WritePin(pSDA_Port, iSDA_Pin, GPIO_PIN_SET);
	I2C_usDelay(1);
	HAL_GPIO_WritePin(pSCL_Port, iSCL_Pin, GPIO_PIN_SET);
	I2C_usDelay(1);
	HAL_GPIO_WritePin(pSCL_Port, iSCL_Pin, GPIO_PIN_RESET);
	I2C_usDelay(1);

	//Data
	for(j=0; j<count; j++)
	{
		for(i=8; i>0; i--)
		{
			d = data[j];
			HAL_GPIO_WritePin(pSDA_Port, iSDA_Pin, (d>>(i-1))&0x0001);
			I2C_usDelay(1);
			HAL_GPIO_WritePin(pSCL_Port, iSCL_Pin, GPIO_PIN_SET);
			I2C_usDelay(1);
			HAL_GPIO_WritePin(pSCL_Port, iSCL_Pin, GPIO_PIN_RESET);
			I2C_usDelay(1);
		}

		//ACK
		HAL_GPIO_WritePin(pSDA_Port, iSDA_Pin, GPIO_PIN_SET);
		I2C_usDelay(1);
		HAL_GPIO_WritePin(pSCL_Port, iSCL_Pin, GPIO_PIN_SET);
		I2C_usDelay(1);
		HAL_GPIO_WritePin(pSCL_Port, iSCL_Pin, GPIO_PIN_RESET);
		I2C_usDelay(1);
	}

	//STOP
	HAL_GPIO_WritePin(pSCL_Port, iSCL_Pin, GPIO_PIN_SET);
	I2C_usDelay(1);
	HAL_GPIO_WritePin(pSDA_Port, iSDA_Pin, GPIO_PIN_SET);
	I2C_usDelay(1);
}


void I2C_usDelay(uint32_t t)
{
	/*uint32_t cptus, cpt;

	for(uint32_t cpt = t; cpt--; cpt>0)
	{
		//cptus = t/BASE_TIME_US;
		//while(cptus--);
	}*/
}
