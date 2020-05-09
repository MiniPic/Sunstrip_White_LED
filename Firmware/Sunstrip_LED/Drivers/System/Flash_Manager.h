/**
  ******************************************************************************
  * @file           : Flash_Manager.h
  * @brief          : Manage Flash (Read, Write, Erase, ...)
  ******************************************************************************
  * @attention
  *	Only for STM32F103C8T6
  *
  ******************************************************************************
  */

#ifndef __SYSTEM_FLASH_MANAGER__H__
#define __SYSTEM_FLASH_MANAGER__H__

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Definitions ------------------------------------------------------------------*/

/* Types Definitions ---------------------------------------------------------*/
typedef enum {FLASH_NO_ERROR,FLASH_ERROR_GENERAL,FLASH_ERROR_ADDRESS,FLASH_ERASE_ERROR,FLASH_WRITE_ERROR
}FlashManager_Error_Code;

/* Private Definitions ------------------------------------------------------------------*/
#define SYSTEM_FLASH_ADDRESS_BEGIN			0x08000000
#define SYSTEM_FLASH_ADDRESS_END			0x08010000

/* Public function prototypes -----------------------------------------------*/
/**
  * @brief  Erase page corresponding address
  * @param address: Flash address word
  */
FlashManager_Error_Code FlashManager_ErasePage(uint32_t address);

/**
  * @brief  Write 32bits word in flash
  * @param address: Flash address word
  * @param data: 32 bits data
  */
FlashManager_Error_Code FlashManager_WriteInt32(uint32_t address, uint32_t data);

/**
  * @brief  Write some 32bits word in flash
  * @param address: Flash address word
  * @param NbWord: Number of word to write
  * @param data: data array
  */
FlashManager_Error_Code FlashManager_WriteMulti(uint32_t address, uint32_t NbWord, uint32_t* data);

/**
  * @brief  Read 32bits word in flash
  * @param address: Flash address word
  */
uint32_t FlashManager_ReadInt32(uint32_t address);

#endif /* __SYSTEM_FLASH_MANAGER__H__ */
