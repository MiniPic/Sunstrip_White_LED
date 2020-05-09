/**
  ******************************************************************************
  * @file           : Flash_Manager.c
  * @brief          : Manage Flash (Read, Write, Erase, ...)
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "Flash_Manager.h"

/* Private variables ---------------------------------------------------------*/
uint32_t flash_temp_data[FLASH_PAGE_SIZE/4];

/* Private function -----------------------------------------------*/
uint32_t FlashManager_PageAddress(uint32_t address)
{
	uint32_t begin_page_address;
	uint32_t offset_page_address;

	if(address>=SYSTEM_FLASH_ADDRESS_BEGIN && address<SYSTEM_FLASH_ADDRESS_END)
	{
		offset_page_address=address%FLASH_PAGE_SIZE;
		begin_page_address=address-offset_page_address;
		return begin_page_address;
	}
	else
		return FLASH_ERROR_ADDRESS;
}

/* Public function -----------------------------------------------*/
FlashManager_Error_Code FlashManager_ErasePage(uint32_t address)
{
	uint32_t errorcode;
	uint32_t SectorError = 0;
	FLASH_EraseInitTypeDef EraseInitStruct;

	EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.Banks = FLASH_BANK_1;
	EraseInitStruct.PageAddress = FlashManager_PageAddress(address);
	EraseInitStruct.NbPages = 1;

	errorcode=HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError);
	if(errorcode!=HAL_OK)
		return FLASH_ERASE_ERROR;
	else
		return FLASH_NO_ERROR;
}

FlashManager_Error_Code FlashManager_WriteInt32(uint32_t address, uint32_t data)
{
	uint32_t cpt_address;
	uint32_t begin_page_address;
	uint32_t offset_page_address;
	uint32_t pWriteFlash;
	__IO uint32_t* pReadFlash;

	if(address>=SYSTEM_FLASH_ADDRESS_BEGIN && address<SYSTEM_FLASH_ADDRESS_END && address%4==0)
	{
		offset_page_address=address%FLASH_PAGE_SIZE;
		begin_page_address=address-offset_page_address;

		//Read page
		for(cpt_address=0; cpt_address<(FLASH_PAGE_SIZE/4) ;cpt_address++)
		{
			pReadFlash = begin_page_address+(cpt_address*4);
			flash_temp_data[cpt_address]=*pReadFlash;
		}

		//Erase page
		HAL_FLASH_Unlock();
		FlashManager_ErasePage(address);

		//Change Data
		flash_temp_data[offset_page_address/4]=data;

		//Write page
		for(cpt_address=0; cpt_address<(FLASH_PAGE_SIZE/4) ;cpt_address++)
		{
			pWriteFlash = begin_page_address+(cpt_address*4);
			if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, pWriteFlash, flash_temp_data[cpt_address]) != HAL_OK)
				return FLASH_WRITE_ERROR;
		}

		HAL_FLASH_Lock();

		return FLASH_NO_ERROR;
	}
	else
		return FLASH_ERROR_ADDRESS;
}

FlashManager_Error_Code FlashManager_WriteMulti(uint32_t address, uint32_t NbWord, uint32_t* data)
{
	uint32_t cpt_address;
	uint32_t begin_page_address;
	uint32_t offset_page_address;
	uint32_t pWriteFlash;
	__IO uint32_t* pReadFlash;

	if(address>=SYSTEM_FLASH_ADDRESS_BEGIN && address<SYSTEM_FLASH_ADDRESS_END && address%4==0)
	{
		offset_page_address=address%FLASH_PAGE_SIZE;
		begin_page_address=address-offset_page_address;

		//Read page
		for(cpt_address=0; cpt_address<(FLASH_PAGE_SIZE/4) ;cpt_address++)
		{
			pReadFlash = begin_page_address+(cpt_address*4);
			flash_temp_data[cpt_address]=*pReadFlash;
		}

		//Erase page
		HAL_FLASH_Unlock();
		FlashManager_ErasePage(address);

		//Change Data
		for(cpt_address=0; cpt_address<NbWord ;cpt_address++)
		{
			flash_temp_data[(offset_page_address/4)+cpt_address]=data[cpt_address];
		}

		//Write page
		for(cpt_address=0; cpt_address<(FLASH_PAGE_SIZE/4) ;cpt_address++)
		{
			pWriteFlash = begin_page_address+(cpt_address*4);
			if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, pWriteFlash, flash_temp_data[cpt_address]) != HAL_OK)
				return FLASH_WRITE_ERROR;
		}

		HAL_FLASH_Lock();

		return FLASH_NO_ERROR;
	}
	else
		return FLASH_ERROR_ADDRESS;
}

uint32_t FlashManager_ReadInt32(uint32_t address)
{
	__IO uint32_t* pReadFlash;

	pReadFlash = address;
	return *pReadFlash;
}
