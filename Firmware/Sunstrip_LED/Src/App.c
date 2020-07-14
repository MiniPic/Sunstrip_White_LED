/**
  ******************************************************************************
  * @file           : App.c
  * @brief          : Application
  ******************************************************************************
  * @attention
  * - works with FreeRTOS
  * - use dmx callback in uart IT
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "../Middlewares/Protocol/DMX/DMX.h"
#include "../Drivers/Devices/Display/ssd1306.h"
#include "../Drivers/System/Flash_Manager.h"
#include "../Drivers/Devices/PWM/PWM.h"
#include "cmsis_os.h"
#include "App.h"
#include "main.h"
#include "stdio.h"

/* Macros ---------------------------------------------------------------*/
#define 	ParamExist()				(FlashManager_ReadInt32(PARAM_EXIST_ADDRESS)==PARAM_EXIST_CODE)

/* Definitions ---------------------------------------------------------------*/
#define 	PARAM_EXIST_ADDRESS			0x0800F800
#define 	PARAM_DMX_PARAM				0x0800F804
#define		PARAM_EXIST_CODE			0x55AA00FF

#define		LED_PWM_PERIOD_VALUE		255
#define		TIMOUT_DMX_SIGNAL			3000			//in ms

#define		TASK_DELAY_LED				10
#define		TASK_DELAY_IHM				50
#define		REFRESH_DISPLAY				5				//5*TASK_DELAY_IHM

#define 	DELAY_SAVE_PARAM			500			    //in tick

#define		TIME_LONG_BP				50				//in tick

#define		__TRUE						0x01
#define 	__FALSE						0x00

#define 	MIN_LED_PWM_VALUE			63
#define 	MAX_LED_PWM_VALUE			244


/* Types Definitions ---------------------------------------------------------*/
typedef enum {	MODE_OFF = 0x00,
				MODE_DMX = 0x01,
				MODE_MANU = 0x02
}App_Mode;

typedef enum {	DISP_PARAM= 0x00,
				DISP_CONFIG_MODE = 0x01,
				DISP_CONFIG_INVERT = 0x02,
				DISP_CONFIG_ADDRESS = 0x03,
				DISP_CONFIG_MANVALUE = 0x04
}Display_Mode_t;


typedef enum {	BP_OFF,
				BP_CLICK,
				BP_1s,
				BP_IDLE,
}BP_Status;

/* Private variables ---------------------------------------------------------*/
osThreadId 			AppLEDTaskHandle;
osThreadId 			AppIHMTaskHandle;

uint16_t			DMX_Adress;

uint8_t				DMX_values[12];
uint8_t				Manu_value;
uint8_t				DMX_signal_OK;
App_Mode			Current_Mode;
uint8_t				IsInverted;
Display_Mode_t		Current_Display;
uint8_t				Display_Cursor;

BP_Status 			Bp_Up;
BP_Status 			Bp_Down;
BP_Status 			Bp_Ok;

/* Private function -----------------------------------------------*/
uint8_t Load_Param()
{
	uint32_t val_param;

	if(ParamExist())
	{
		val_param = 	FlashManager_ReadInt32(PARAM_DMX_PARAM);
		IsInverted = 	(val_param&0xF0000000)>>28;
		Current_Mode = 	(val_param&0x0F000000)>>24;
		Manu_value = 	(val_param&0x00FF0000)>>16;
		DMX_Adress = 	(val_param&0x0000FFFF);
		return __TRUE;
	}
	else
	{
		Current_Mode = MODE_OFF;
		IsInverted = __FALSE;
		Manu_value = 100;
		DMX_Adress = 1;
		return __FALSE;
	}
}

void Write_Param()
{
	uint32_t data[2];
	data[0] = PARAM_EXIST_CODE;
	data[1] = 0x00;
	data[1] |= (IsInverted<<28);
	data[1] |= (Current_Mode<<24);
	data[1] |= (Manu_value<<16);
	data[1] |= (DMX_Adress);

	FlashManager_WriteMulti(PARAM_EXIST_ADDRESS,2,data);

//	Display_Cursor=0;
}

void Update_Display()
{
	char Str_percent[]="000";
    char Str_dmx[]="000";
    char Str_add[]="000";
    uint8_t percent_value;

    SSD1306_Clear();

    if(Current_Display == DISP_PARAM)
    {
    	//CURSOR
    	SSD1306_DrawFilledTriangle(0, 11*Display_Cursor, 0, 11*Display_Cursor + 8, 7, 11*Display_Cursor + 4, 1);

    	//MODE
    	SSD1306_GotoXY (10,0);
    	SSD1306_Puts ("MODE: ", &Font_7x10, 1);
    	SSD1306_GotoXY (80,0);
    	if(Current_Mode == MODE_OFF)
    		SSD1306_Puts ("OFF", &Font_7x10, 1);
    	if(Current_Mode == MODE_MANU)
    		SSD1306_Puts ("MANU", &Font_7x10, 1);
    	if(Current_Mode == MODE_DMX)
    	    SSD1306_Puts ("DMX", &Font_7x10, 1);

    	//INVERT
    	SSD1306_GotoXY (10,11);
    	SSD1306_Puts ("INVERT: ", &Font_7x10, 1);
    	SSD1306_GotoXY (80,11);
		if(IsInverted)
			SSD1306_Puts ("YES", &Font_7x10, 1);
		else
			SSD1306_Puts ("NO", &Font_7x10, 1);

    	//ADRESS DMX
    	SSD1306_GotoXY (10,22);
    	SSD1306_Puts ("ADDRESS: ", &Font_7x10, 1);
    	SSD1306_GotoXY (80,22);
    	sprintf(Str_add,"%03d",DMX_Adress);
    	SSD1306_Puts (Str_add, &Font_7x10, 1);

    	//MAN VALUE
    	SSD1306_GotoXY (10,33);
    	SSD1306_Puts ("MAN VALUE: ", &Font_7x10, 1);
    	SSD1306_GotoXY (80,33);
		sprintf(Str_add,"%03d",Manu_value);
		SSD1306_Puts (Str_add, &Font_7x10, 1);
    }
    else if(Current_Display == DISP_CONFIG_MODE)
    {
    	SSD1306_GotoXY (40,0);

    	switch(Current_Mode)
		{
		case MODE_OFF:
			SSD1306_Puts ("OFF", &Font_16x26, 1);
			break;
		case MODE_MANU:
			SSD1306_Puts ("MANU", &Font_16x26, 1);
			break;
		case MODE_DMX:
			SSD1306_Puts ("DMX", &Font_16x26, 1);
			break;
		default:
			break;
		}
    }
    else if(Current_Display == DISP_CONFIG_INVERT)
	{

    	if(IsInverted)
    	{
    		SSD1306_GotoXY (40,0);
    		SSD1306_Puts ("YES", &Font_16x26, 1);
    	}
    	else
    	{
    		SSD1306_GotoXY (48,0);
    		SSD1306_Puts ("NO", &Font_16x26, 1);
    	}
	}
    else if(Current_Display == DISP_CONFIG_MANVALUE)		//MANU
    {
    	percent_value = Manu_value;
    	sprintf(Str_percent,"%03d",percent_value);
    	SSD1306_GotoXY (25,0);
    	SSD1306_Puts (Str_percent, &Font_16x26, 1);
    	SSD1306_GotoXY (74,0);
    	SSD1306_Puts ("%", &Font_16x26, 1);
    	SSD1306_GotoXY (0, 45);
    	SSD1306_Puts ("MODE:MANUAL", &Font_11x18, 1);
    }
    else if (Current_Display == DISP_CONFIG_ADDRESS)		//DMX
    {
    	percent_value = (DMX_values[0]*100/255);
    	sprintf(Str_add,"%03d",DMX_Adress);
    	if(DMX_signal_OK)
		{
			SSD1306_GotoXY (0, 45);
			SSD1306_Puts ("SIGNAL :OK", &Font_11x18, 1);
			sprintf(Str_percent,"%03d",percent_value);
			sprintf(Str_dmx,"%03d",DMX_values[0]);
		}
		else
		{
			SSD1306_GotoXY (0, 45);
			SSD1306_Puts ("SIGNAL :ERR", &Font_11x18, 1);
			sprintf(Str_percent,"---");
			sprintf(Str_dmx,"---");
		}
    	SSD1306_GotoXY (25,0);
		SSD1306_Puts (Str_percent, &Font_16x26, 1);
		SSD1306_GotoXY (74,0);
		SSD1306_Puts ("%", &Font_16x26, 1);
		SSD1306_GotoXY (0, 27);
		SSD1306_Puts ("DMX ADD:", &Font_11x18, 1);
		SSD1306_GotoXY (89, 27);
		SSD1306_Puts (Str_add, &Font_11x18, 1);
		SSD1306_GotoXY (105, 0);
		SSD1306_Puts ("DMX", &Font_7x10, 1);
		SSD1306_GotoXY (105, 10);
		SSD1306_Puts (Str_dmx, &Font_7x10, 1);
    }
	SSD1306_UpdateScreen(); //display
}

void Manage_Button()
{
	static uint32_t time_BpUp=0;
	static uint32_t time_BpDown=0;
	static uint32_t time_BpOk=0;

	//UP
	if(!HAL_GPIO_ReadPin(T1_GPIO_Port, T1_Pin))
	{
		if(Bp_Up==BP_OFF)
		{
			Bp_Up=BP_CLICK;
			time_BpUp = HAL_GetTick();
		}
		else if((HAL_GetTick() > time_BpUp+TIME_LONG_BP) && Bp_Up==BP_IDLE)
		{
			time_BpUp = HAL_GetTick();
			Bp_Up=BP_1s;
		}
		else
			Bp_Up = BP_IDLE;
	}
	else
		Bp_Up=BP_OFF;

	//DOWN
	if(!HAL_GPIO_ReadPin(T3_GPIO_Port, T3_Pin))
	{
		if(Bp_Down==BP_OFF)
		{
			Bp_Down=BP_CLICK;
			time_BpDown = HAL_GetTick();
		}
		else if((HAL_GetTick() > time_BpDown+TIME_LONG_BP) && Bp_Down==BP_IDLE)
		{
			time_BpDown = HAL_GetTick();
			Bp_Down=BP_1s;
		}
		else
			Bp_Down = BP_IDLE;
	}
	else
		Bp_Down=BP_OFF;

	//OK
	if(!HAL_GPIO_ReadPin(T2_GPIO_Port, T2_Pin))
	{
		if(Bp_Ok==BP_OFF)
		{
			Bp_Ok=BP_CLICK;
			time_BpOk = HAL_GetTick();
		}
		else if((HAL_GetTick()>time_BpOk+TIME_LONG_BP) && Bp_Ok==BP_IDLE)
		{
			time_BpOk = HAL_GetTick();
			Bp_Ok=BP_1s;
		}
		else
			Bp_Ok = BP_IDLE;
	}
	else
		Bp_Ok=BP_OFF;
}

void AppLEDTask(void const * argument)
{
	static uint8_t A;
	for(;;)
	{
		osDelay(1000);//(TASK_DELAY_LED);

		A++;
		if(A>=10)
			A=0;

		/*if(Current_Mode == MODE_OFF)
			PWM_SetDuty(LED1_pwmtimer,LED1_PWMchannel,0);
		else if(Current_Mode == MODE_MANU)
			PWM_SetDuty(LED1_pwmtimer,LED1_PWMchannel,(uint32_t)Manu_value*255/100);
		else
		{
			DMX_values[0] = Protocol_DMX_GetValue(1);									//CHANNEL 1
			if(HAL_GetTick()>Protocol_DMX_GetLastTickFrame()+TIMOUT_DMX_SIGNAL)
			{
				DMX_values[0]=0;				//OFF LED if no signal
				DMX_signal_OK = __FALSE;
			}
			else
			{
				DMX_signal_OK = __TRUE;
			}
			PWM_SetDuty(LED1_pwmtimer,LED1_PWMchannel,(uint32_t)DMX_values[0]);
		}*/

		switch(A)
		{
			case 0:
				SSD1306_Clear();
				SSD1306_GotoXY (40,0);
				    		SSD1306_Puts ("1", &Font_16x26, 1);
				    		SSD1306_UpdateScreen(); //display
					PWM_SetDuty(LED1_pwmtimer,LED1_PWMchannel,10);		//PWM Off
					PWM_SetDuty(LED2_pwmtimer,LED2_PWMchannel,0);		//PWM Off
					PWM_SetDuty(LED3_pwmtimer,LED3_PWMchannel,0);		//PWM Off
					PWM_SetDuty(LED4_pwmtimer,LED4_PWMchannel,0);		//PWM Off
					PWM_SetDuty(LED5_pwmtimer,LED5_PWMchannel,0);		//PWM Off
					PWM_SetDuty(LED6_pwmtimer,LED6_PWMchannel,0);		//PWM Off
					PWM_SetDuty(LED7_pwmtimer,LED7_PWMchannel,0);		//PWM Off
					PWM_SetDuty(LED8_pwmtimer,LED8_PWMchannel,0);		//PWM Off
					PWM_SetDuty(LED9_pwmtimer,LED9_PWMchannel,0);		//PWM Off
					PWM_SetDuty(LED10_pwmtimer,LED10_PWMchannel,0);		//PWM Off
					break;
			case 1:
				SSD1306_Clear();
								SSD1306_GotoXY (40,0);
								    		SSD1306_Puts ("2", &Font_16x26, 1);
								    		SSD1306_UpdateScreen(); //display
								    		PWM_SetDuty(LED1_pwmtimer,LED1_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED2_pwmtimer,LED2_PWMchannel,10);		//PWM Off
								    		PWM_SetDuty(LED3_pwmtimer,LED3_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED4_pwmtimer,LED4_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED5_pwmtimer,LED5_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED6_pwmtimer,LED6_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED7_pwmtimer,LED7_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED8_pwmtimer,LED8_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED9_pwmtimer,LED9_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED10_pwmtimer,LED10_PWMchannel,0);		//PWM Off
					break;
			case 2:
				SSD1306_Clear();
								SSD1306_GotoXY (40,0);
								    		SSD1306_Puts ("3", &Font_16x26, 1);
								    		SSD1306_UpdateScreen(); //display
								    		PWM_SetDuty(LED1_pwmtimer,LED1_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED2_pwmtimer,LED2_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED3_pwmtimer,LED3_PWMchannel,10);		//PWM Off
								    		PWM_SetDuty(LED4_pwmtimer,LED4_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED5_pwmtimer,LED5_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED6_pwmtimer,LED6_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED7_pwmtimer,LED7_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED8_pwmtimer,LED8_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED9_pwmtimer,LED9_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED10_pwmtimer,LED10_PWMchannel,0);		//PWM Off
					break;
			case 3:
				SSD1306_Clear();
								SSD1306_GotoXY (40,0);
								    		SSD1306_Puts ("4", &Font_16x26, 1);
								    		SSD1306_UpdateScreen(); //display
								    		PWM_SetDuty(LED1_pwmtimer,LED1_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED2_pwmtimer,LED2_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED3_pwmtimer,LED3_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED4_pwmtimer,LED4_PWMchannel,10);		//PWM Off
								    		PWM_SetDuty(LED5_pwmtimer,LED5_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED6_pwmtimer,LED6_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED7_pwmtimer,LED7_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED8_pwmtimer,LED8_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED9_pwmtimer,LED9_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED10_pwmtimer,LED10_PWMchannel,0);		//PWM Off
					break;
			case 4:
				SSD1306_Clear();
								SSD1306_GotoXY (40,0);
								    		SSD1306_Puts ("5", &Font_16x26, 1);
								    		SSD1306_UpdateScreen(); //display
								    		PWM_SetDuty(LED1_pwmtimer,LED1_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED2_pwmtimer,LED2_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED3_pwmtimer,LED3_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED4_pwmtimer,LED4_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED5_pwmtimer,LED5_PWMchannel,10);		//PWM Off
								    		PWM_SetDuty(LED6_pwmtimer,LED6_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED7_pwmtimer,LED7_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED8_pwmtimer,LED8_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED9_pwmtimer,LED9_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED10_pwmtimer,LED10_PWMchannel,0);		//PWM Off
					break;
			case 5:
				SSD1306_Clear();
								SSD1306_GotoXY (40,0);
								    		SSD1306_Puts ("6", &Font_16x26, 1);
								    		SSD1306_UpdateScreen(); //display
								    		PWM_SetDuty(LED1_pwmtimer,LED1_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED2_pwmtimer,LED2_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED3_pwmtimer,LED3_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED4_pwmtimer,LED4_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED5_pwmtimer,LED5_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED6_pwmtimer,LED6_PWMchannel,10);		//PWM Off
								    		PWM_SetDuty(LED7_pwmtimer,LED7_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED8_pwmtimer,LED8_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED9_pwmtimer,LED9_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED10_pwmtimer,LED10_PWMchannel,0);		//PWM Off
					break;
			case 6:
				SSD1306_Clear();
								SSD1306_GotoXY (40,0);
								    		SSD1306_Puts ("7", &Font_16x26, 1);
								    		SSD1306_UpdateScreen(); //display
								    		PWM_SetDuty(LED1_pwmtimer,LED1_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED2_pwmtimer,LED2_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED3_pwmtimer,LED3_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED4_pwmtimer,LED4_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED5_pwmtimer,LED5_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED6_pwmtimer,LED6_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED7_pwmtimer,LED7_PWMchannel,10);		//PWM Off
								    		PWM_SetDuty(LED8_pwmtimer,LED8_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED9_pwmtimer,LED9_PWMchannel,0);		//PWM Off
					PWM_SetDuty(LED10_pwmtimer,LED10_PWMchannel,0);		//PWM Off
					break;
			case 7:
				SSD1306_Clear();
								SSD1306_GotoXY (40,0);
								    		SSD1306_Puts ("8", &Font_16x26, 1);
								    		SSD1306_UpdateScreen(); //display
								    		PWM_SetDuty(LED1_pwmtimer,LED1_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED2_pwmtimer,LED2_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED3_pwmtimer,LED3_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED4_pwmtimer,LED4_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED5_pwmtimer,LED5_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED6_pwmtimer,LED6_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED7_pwmtimer,LED7_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED8_pwmtimer,LED8_PWMchannel,10);		//PWM Off
								    		PWM_SetDuty(LED9_pwmtimer,LED9_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED10_pwmtimer,LED10_PWMchannel,0);		//PWM Off
					break;
			case 8:
				SSD1306_Clear();
								SSD1306_GotoXY (40,0);
								    		SSD1306_Puts ("9", &Font_16x26, 1);
								    		SSD1306_UpdateScreen(); //display
								    		PWM_SetDuty(LED1_pwmtimer,LED1_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED2_pwmtimer,LED2_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED3_pwmtimer,LED3_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED4_pwmtimer,LED4_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED5_pwmtimer,LED5_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED6_pwmtimer,LED6_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED7_pwmtimer,LED7_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED8_pwmtimer,LED8_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED9_pwmtimer,LED9_PWMchannel,10);		//PWM Off
								    		PWM_SetDuty(LED10_pwmtimer,LED10_PWMchannel,0);		//PWM Off
					break;
			case 9:
				SSD1306_Clear();
								SSD1306_GotoXY (40,0);
								    		SSD1306_Puts ("10", &Font_16x26, 1);
								    		SSD1306_UpdateScreen(); //display
								    		PWM_SetDuty(LED1_pwmtimer,LED1_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED2_pwmtimer,LED2_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED3_pwmtimer,LED3_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED4_pwmtimer,LED4_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED5_pwmtimer,LED5_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED6_pwmtimer,LED6_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED7_pwmtimer,LED7_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED8_pwmtimer,LED8_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED9_pwmtimer,LED9_PWMchannel,0);		//PWM Off
								    		PWM_SetDuty(LED10_pwmtimer,LED10_PWMchannel,10);		//PWM Off
					break;
		}
	}
}

void AppIHMTask(void const * argument)
{
	uint32_t cpt_refresh=0;
	uint32_t tick_save_param;
	uint32_t param_changed = __FALSE;

	for(;;)
	{
		osDelay(TASK_DELAY_IHM);

		cpt_refresh++;
		if(cpt_refresh>=REFRESH_DISPLAY)
		{
			//Update_Display();
			cpt_refresh=0;
		}

		Manage_Button();

		if(Current_Display == DISP_PARAM)
		{
			if(Bp_Up == BP_CLICK)
			{
				if(Display_Cursor == 3)
					Display_Cursor = 0;
				else
					Display_Cursor++;
			}
			if(Bp_Down == BP_CLICK)
			{
				if(Display_Cursor == 0)
					Display_Cursor = 3;
				else
					Display_Cursor--;
			}
			if(Bp_Ok == BP_CLICK)
			{
				switch(Display_Cursor)
				{
				case 0:
					Current_Display = DISP_CONFIG_MODE;
					break;
				case 1:
					Current_Display = DISP_CONFIG_INVERT;
					break;
				case 2:
					Current_Display = DISP_CONFIG_ADDRESS;
					break;
				case 3:
					Current_Display = DISP_CONFIG_MANVALUE;
					break;
				default:
					break;
				}
			}
		}

		else if(Current_Display == DISP_CONFIG_MODE)
		{
			if(Bp_Up == BP_CLICK)
			{
				param_changed=__TRUE;
				tick_save_param = HAL_GetTick();
				if(Current_Mode == 2)
					Current_Mode = 0;
				else
					Current_Mode++;
			}
			if(Bp_Down == BP_CLICK)
			{
				param_changed=__TRUE;
				tick_save_param = HAL_GetTick();
				if(Current_Mode == 0)
					Current_Mode = 2;
				else
					Current_Mode--;
			}

			if(Bp_Ok == BP_CLICK)
			{
				Current_Display = DISP_PARAM;
			}
		}

		else if(Current_Display == DISP_CONFIG_ADDRESS)
		{
			if(Bp_Up == BP_CLICK)
			{
				if(DMX_Adress<512)
					DMX_Adress++;
				param_changed = __TRUE;
				tick_save_param = HAL_GetTick();
				Protocol_DMX_init(DMX_Adress,DMX_uart);
			}
			if(Bp_Up == BP_1s)
			{
				if(DMX_Adress<=502)
					DMX_Adress+=10;
				else
					DMX_Adress=512;
				param_changed = __TRUE;
				tick_save_param = HAL_GetTick();
				Protocol_DMX_init(DMX_Adress,DMX_uart);
			}
			if(Bp_Down == BP_CLICK)
			{
				if(DMX_Adress>1)
					DMX_Adress--;
				param_changed = __TRUE;
				tick_save_param = HAL_GetTick();
				Protocol_DMX_init(DMX_Adress,DMX_uart);
			}
			if(Bp_Down == BP_1s)
			{
				if(DMX_Adress>=11)
					DMX_Adress-=10;
				else
					DMX_Adress=1;
				param_changed = __TRUE;
				tick_save_param = HAL_GetTick();
				Protocol_DMX_init(DMX_Adress,DMX_uart);
			}
			if(Bp_Ok == BP_CLICK)
			{
				Current_Display = DISP_PARAM;
			}
		}

		else if(Current_Display==DISP_CONFIG_MANVALUE)
		{
			if(Bp_Up == BP_CLICK)
			{
				if(Manu_value<100)
					Manu_value++;
				param_changed = __TRUE;
				tick_save_param = HAL_GetTick();
			}
			if(Bp_Up == BP_1s)
			{
				if(Manu_value<=90)
					Manu_value+=10;
				else
					Manu_value=100;
				param_changed = __TRUE;
				tick_save_param = HAL_GetTick();
			}
			if(Bp_Down == BP_CLICK)
			{
				if(Manu_value>0)
					Manu_value--;
				param_changed = __TRUE;
				tick_save_param = HAL_GetTick();
			}
			if(Bp_Down == BP_1s)
			{
				if(Manu_value>=10)
					Manu_value-=10;
				else
					Manu_value=0;
				param_changed = __TRUE;
				tick_save_param = HAL_GetTick();
			}
			if(Bp_Ok == BP_CLICK)
			{
				Current_Display = DISP_PARAM;
			}
		}

		else if(Current_Display==DISP_CONFIG_INVERT)
		{
			if(Bp_Up == BP_CLICK || Bp_Down == BP_CLICK)
			{
				param_changed=__TRUE;
				tick_save_param = HAL_GetTick();
				if(IsInverted)
					IsInverted = __FALSE;
				else
					IsInverted = __TRUE;
			}
			if(Bp_Ok == BP_CLICK)
			{
				Current_Display = DISP_PARAM;
			}
		}

		if(HAL_GetTick()>(tick_save_param+DELAY_SAVE_PARAM) && param_changed==__TRUE)
		{
			param_changed = __FALSE;
			Write_Param();
		}
	}
}

/* Public function -----------------------------------------------*/
void App_Init()
{
	Load_Param();
	Current_Display = DISP_PARAM;
	Display_Cursor = 0;

	DMX_signal_OK = __FALSE;

	//PWM_SetPWM(LED1_pwmtimer,LED1_PWMchannel,LED_PWM_PERIOD_VALUE,10);		//PWM Off
	//PWM_SetPWM(LED2_pwmtimer,LED2_PWMchannel,LED_PWM_PERIOD_VALUE,10);		//PWM Off
	//PWM_SetPWM(LED3_pwmtimer,LED3_PWMchannel,LED_PWM_PERIOD_VALUE,10);		//PWM Off
	//PWM_SetPWM(LED4_pwmtimer,LED4_PWMchannel,LED_PWM_PERIOD_VALUE,10);		//PWM Off
	//PWM_SetPWM(LED5_pwmtimer,LED5_PWMchannel,LED_PWM_PERIOD_VALUE,10);		//PWM Off
	//PWM_SetPWM(LED6_pwmtimer,LED6_PWMchannel,LED_PWM_PERIOD_VALUE,10);		//PWM Off
	//PWM_SetPWM(LED7_pwmtimer,LED7_PWMchannel,LED_PWM_PERIOD_VALUE,10);		//PWM Off
	//PWM_SetPWM(LED8_pwmtimer,LED8_PWMchannel,LED_PWM_PERIOD_VALUE,10);		//PWM Off
	//PWM_SetPWM(LED9_pwmtimer,LED9_PWMchannel,LED_PWM_PERIOD_VALUE,10);		//PWM Off
	//PWM_SetPWM(LED10_pwmtimer,LED10_PWMchannel,LED_PWM_PERIOD_VALUE,10);		//PWM Off

	HAL_TIM_PWM_Start( LED1_pwmtimer, LED1_PWMchannel );
	HAL_TIM_PWM_Start( LED2_pwmtimer, LED2_PWMchannel );
	HAL_TIM_PWM_Start( LED3_pwmtimer, LED3_PWMchannel );
	HAL_TIM_PWM_Start( LED4_pwmtimer, LED4_PWMchannel );
	HAL_TIM_PWM_Start( LED5_pwmtimer, LED5_PWMchannel );
	HAL_TIM_PWM_Start( LED6_pwmtimer, LED6_PWMchannel );
	HAL_TIM_PWM_Start( LED7_pwmtimer, LED7_PWMchannel );
	HAL_TIM_PWM_Start( LED8_pwmtimer, LED8_PWMchannel );
	HAL_TIM_PWM_Start( LED9_pwmtimer, LED9_PWMchannel );
	HAL_TIM_PWM_Start( LED10_pwmtimer, LED10_PWMchannel );

	PWM_SetDuty(LED1_pwmtimer,LED1_PWMchannel,0);		//PWM Off
	PWM_SetDuty(LED2_pwmtimer,LED2_PWMchannel,0);		//PWM Off
	PWM_SetDuty(LED3_pwmtimer,LED3_PWMchannel,0);		//PWM Off
	PWM_SetDuty(LED4_pwmtimer,LED4_PWMchannel,0);		//PWM Off
	PWM_SetDuty(LED5_pwmtimer,LED5_PWMchannel,0);		//PWM Off
	PWM_SetDuty(LED6_pwmtimer,LED6_PWMchannel,0);		//PWM Off
	PWM_SetDuty(LED7_pwmtimer,LED7_PWMchannel,0);		//PWM Off
	PWM_SetDuty(LED8_pwmtimer,LED8_PWMchannel,0);		//PWM Off
	PWM_SetDuty(LED9_pwmtimer,LED9_PWMchannel,0);		//PWM Off
	PWM_SetDuty(LED10_pwmtimer,LED10_PWMchannel,0);		//PWM Off


	Protocol_DMX_init(DMX_Adress,DMX_uart);
	//SSD1306_Init(hi2c_display);  // initialise
	SSD1306_Init();  // initialise

	Bp_Up = BP_OFF;
	Bp_Down = BP_OFF;
	Bp_Ok = BP_OFF;

	CreatAppTasks();
}

void CreatAppTasks (void)
{
	osThreadDef(App_LED_Task, AppLEDTask, osPriorityHigh, 0, 256);
	AppLEDTaskHandle = osThreadCreate(osThread(App_LED_Task), NULL);

	osThreadDef(App_IHM_Task, AppIHMTask, osPriorityNormal, 0, 256);
	AppIHMTaskHandle = osThreadCreate(osThread(App_IHM_Task), NULL);
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	Protocol_DMX_UartCallback(huart);
}
