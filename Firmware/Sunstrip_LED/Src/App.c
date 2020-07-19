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
#include "tim.h"
#include "../Middlewares/Protocol/DMX/DMX.h"
#include "../Drivers/Devices/Display/ssd1306.h"
#include "../Drivers/System/Flash_Manager.h"
#include "../Drivers/Devices/PWM/PWM.h"
#include "../Middlewares/Protocol/GENE_I2C/GENE_I2C_Master.h"
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

#define		REFRESH_DISPLAY				100				//in ms
#define		REFRESH_FAN					100				//in ms

#define 	DELAY_SAVE_PARAM			3000		    //in ms

#define		TIME_LONG_BP				1000			//in ms

#define		__TRUE						0x01
#define 	__FALSE						0x00

#define 	MIN_LED_PWM_VALUE			0
#define 	MAX_LED_PWM_VALUE			215


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
uint16_t			DMX_Adress;

uint8_t				Current_Value[10];
uint8_t				Speed_value;
uint8_t				Strob_value_Full;
uint8_t				Strob_value_Rand;
uint8_t				Manu_value;
uint8_t				DMX_signal_OK;
App_Mode			Current_Mode;
uint8_t				IsInverted;
volatile Display_Mode_t		Current_Display;
uint8_t				Display_Cursor;
uint8_t				Mean_Value;
uint32_t 			tick_save_param;
uint32_t 			param_changed = __FALSE;

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

	__disable_irq();
	FlashManager_WriteMulti(PARAM_EXIST_ADDRESS,2,data);
	__enable_irq();

//	Display_Cursor=0;
}

uint8_t MeanArray(uint8_t array[], uint8_t size)
{
	uint32_t mean = 0;

	for (uint8_t i=0; i<size; i++)
	{
		mean += array[i];
	}

	return mean/size;
}

void PWM_SetDutyAdapt(TIM_HandleTypeDef* timer, uint32_t channel, uint16_t power)
{

	uint32_t value_pulse;

	//out power
	if(power==0)
		value_pulse = 0;
	else
	{
		value_pulse = ((254*power)/(MAX_LED_PWM_VALUE-MIN_LED_PWM_VALUE))+MIN_LED_PWM_VALUE;
		if(value_pulse > MAX_LED_PWM_VALUE)
			value_pulse = MAX_LED_PWM_VALUE;
	}

	 __HAL_TIM_SET_COMPARE(timer, channel, value_pulse);
}

void PatchPWMtoLED (uint8_t isInverted)
{
	if(isInverted)
	{
		  LED1_pwmtimer = &htim3;
		  LED1_PWMchannel = TIM_CHANNEL_2;
		  LED2_pwmtimer = &htim3;
		  LED2_PWMchannel = TIM_CHANNEL_1;
		  LED3_pwmtimer = &htim2;
		  LED3_PWMchannel = TIM_CHANNEL_4;
		  LED4_pwmtimer = &htim2;
		  LED4_PWMchannel = TIM_CHANNEL_3;
		  LED5_pwmtimer = &htim2;
		  LED5_PWMchannel = TIM_CHANNEL_2;
		  LED6_pwmtimer = &htim2;
		  LED6_PWMchannel = TIM_CHANNEL_1;
		  LED7_pwmtimer = &htim1;
		  LED7_PWMchannel = TIM_CHANNEL_1;
		  LED8_pwmtimer = &htim1;
		  LED8_PWMchannel = TIM_CHANNEL_2;
		  LED9_pwmtimer = &htim1;
		  LED9_PWMchannel = TIM_CHANNEL_3;
		  LED10_pwmtimer = &htim1;
		  LED10_PWMchannel = TIM_CHANNEL_4;
	}
	else
	{
		  LED1_pwmtimer = &htim1;
		  LED1_PWMchannel = TIM_CHANNEL_4;
		  LED2_pwmtimer = &htim1;
		  LED2_PWMchannel = TIM_CHANNEL_3;
		  LED3_pwmtimer = &htim1;
		  LED3_PWMchannel = TIM_CHANNEL_2;
		  LED4_pwmtimer = &htim1;
		  LED4_PWMchannel = TIM_CHANNEL_1;
		  LED5_pwmtimer = &htim2;
		  LED5_PWMchannel = TIM_CHANNEL_1;
		  LED6_pwmtimer = &htim2;
		  LED6_PWMchannel = TIM_CHANNEL_2;
		  LED7_pwmtimer = &htim2;
		  LED7_PWMchannel = TIM_CHANNEL_3;
		  LED8_pwmtimer = &htim2;
		  LED8_PWMchannel = TIM_CHANNEL_4;
		  LED9_pwmtimer = &htim3;
		  LED9_PWMchannel = TIM_CHANNEL_1;
		  LED10_pwmtimer = &htim3;
		  LED10_PWMchannel = TIM_CHANNEL_2;
	}
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
    	percent_value = Mean_Value*100/255;
    	sprintf(Str_add,"%03d",DMX_Adress);
    	if(DMX_signal_OK)
		{
			SSD1306_GotoXY (0, 45);
			SSD1306_Puts ("SIGNAL :OK", &Font_11x18, 1);
			sprintf(Str_percent,"%03d",percent_value);
			sprintf(Str_dmx,"%03d",Protocol_DMX_GetValue(1));
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
		SSD1306_Puts ("CH1", &Font_7x10, 1);
		SSD1306_GotoXY (105, 10);
		SSD1306_Puts (Str_dmx, &Font_7x10, 1);
    }

    if(param_changed==__TRUE)
    {
    	SSD1306_GotoXY (120,53);
    	SSD1306_Puts ("M", &Font_7x10, 1);
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

void AppLEDTask()
{
	uint8_t temp_strob_value =0;
	uint8_t rand_value=0;
	int32_t calcul;

	static uint32_t cpt_strob=0;

	//Mean value calculation
	Mean_Value = MeanArray(Current_Value,10);

	if(Current_Mode == MODE_OFF)
	{
		Current_Value[0]=0;
		Current_Value[1]=0;
		Current_Value[2]=0;
		Current_Value[3]=0;
		Current_Value[4]=0;
		Current_Value[5]=0;
		Current_Value[6]=0;
		Current_Value[7]=0;
		Current_Value[8]=0;
		Current_Value[9]=0;

		Speed_value=0;
		Strob_value_Full=0;
		Strob_value_Rand=0;
	}
	else if(Current_Mode == MODE_MANU)
	{
		Current_Value[0]=(uint32_t)(Manu_value*255/100);
		Current_Value[1]=(uint32_t)(Manu_value*255/100);
		Current_Value[2]=(uint32_t)(Manu_value*255/100);
		Current_Value[3]=(uint32_t)(Manu_value*255/100);
		Current_Value[4]=(uint32_t)(Manu_value*255/100);
		Current_Value[5]=(uint32_t)(Manu_value*255/100);
		Current_Value[6]=(uint32_t)(Manu_value*255/100);
		Current_Value[7]=(uint32_t)(Manu_value*255/100);
		Current_Value[8]=(uint32_t)(Manu_value*255/100);
		Current_Value[9]=(uint32_t)(Manu_value*255/100);

		Speed_value=0;
		Strob_value_Full=0;
		Strob_value_Rand=0;
	}
	else	//DMX MODE
	{
		if(HAL_GetTick()>Protocol_DMX_GetLastTickFrame()+TIMOUT_DMX_SIGNAL)
		{
			Current_Value[0]=0;
			Current_Value[1]=0;
			Current_Value[2]=0;
			Current_Value[3]=0;
			Current_Value[4]=0;
			Current_Value[5]=0;
			Current_Value[6]=0;
			Current_Value[7]=0;
			Current_Value[8]=0;
			Current_Value[9]=0;

			Speed_value=0;
			Strob_value_Full=0;
			Strob_value_Rand=0;

			DMX_signal_OK = __FALSE;
		}
		else
		{
			Speed_value=Protocol_DMX_GetValue(12);
			temp_strob_value=Protocol_DMX_GetValue(11);

			//Speed update value
			if(Speed_value==0)		//Instant
			{
				Current_Value[0] = Protocol_DMX_GetValue(1);									//CHANNEL 1
				Current_Value[1] = Protocol_DMX_GetValue(2);									//CHANNEL 2
				Current_Value[2] = Protocol_DMX_GetValue(3);									//CHANNEL 3
				Current_Value[3] = Protocol_DMX_GetValue(4);									//CHANNEL 4
				Current_Value[4] = Protocol_DMX_GetValue(5);									//CHANNEL 5
				Current_Value[5] = Protocol_DMX_GetValue(6);									//CHANNEL 6
				Current_Value[6] = Protocol_DMX_GetValue(7);									//CHANNEL 7
				Current_Value[7] = Protocol_DMX_GetValue(8);									//CHANNEL 8
				Current_Value[8] = Protocol_DMX_GetValue(9);									//CHANNEL 9
				Current_Value[9] = Protocol_DMX_GetValue(10);									//CHANNEL 10
			}
			else
			{
				for (uint8_t i =0 ;i<10; i++)
				{
					if(Current_Value[i]<Protocol_DMX_GetValue(i+1))
					{
						calcul=Current_Value[i]+(260-Speed_value)/5;
						if(calcul>Protocol_DMX_GetValue(i+1))
							Current_Value[i]=Protocol_DMX_GetValue(i+1);
						else
							Current_Value[i]=calcul;
					}
					else if (Current_Value[i]>Protocol_DMX_GetValue(i+1))
					{
						calcul=Current_Value[i]-(260-Speed_value)/5;
						if(calcul<Protocol_DMX_GetValue(i+1))
							Current_Value[i]=Protocol_DMX_GetValue(i+1);
						else
							Current_Value[i]=calcul;
					}
				}
			}

			//Strob decomposition channel
			if(temp_strob_value>127)
			{
				Strob_value_Full=0;
				Strob_value_Rand=temp_strob_value-127;
			}
			else
			{
				Strob_value_Full=temp_strob_value;
				Strob_value_Rand=0;
			}
			DMX_signal_OK = __TRUE;
		}

		//Set value on LED
		if(temp_strob_value>0)	//STROB
		{
			if(Strob_value_Full>0 && cpt_strob>=(129-Strob_value_Full)/2)
			{
				PWM_SetDutyAdapt(LED1_pwmtimer,LED1_PWMchannel,(uint32_t)Current_Value[0]);
				PWM_SetDutyAdapt(LED2_pwmtimer,LED2_PWMchannel,(uint32_t)Current_Value[1]);
				PWM_SetDutyAdapt(LED3_pwmtimer,LED3_PWMchannel,(uint32_t)Current_Value[2]);
				PWM_SetDutyAdapt(LED4_pwmtimer,LED4_PWMchannel,(uint32_t)Current_Value[3]);
				PWM_SetDutyAdapt(LED5_pwmtimer,LED5_PWMchannel,(uint32_t)Current_Value[4]);
				PWM_SetDutyAdapt(LED6_pwmtimer,LED6_PWMchannel,(uint32_t)Current_Value[5]);
				PWM_SetDutyAdapt(LED7_pwmtimer,LED7_PWMchannel,(uint32_t)Current_Value[6]);
				PWM_SetDutyAdapt(LED8_pwmtimer,LED8_PWMchannel,(uint32_t)Current_Value[7]);
				PWM_SetDutyAdapt(LED9_pwmtimer,LED9_PWMchannel,(uint32_t)Current_Value[8]);
				PWM_SetDutyAdapt(LED10_pwmtimer,LED10_PWMchannel,(uint32_t)Current_Value[9]);

				cpt_strob=0;
			}
			else if(Strob_value_Rand>0 && cpt_strob>=(128-Strob_value_Rand)/2)
			{
				rand_value = HAL_GetTick()%10;
				if(rand_value==0) PWM_SetDutyAdapt(LED1_pwmtimer,LED1_PWMchannel,(uint32_t)Current_Value[0]); else PWM_SetDutyAdapt(LED1_pwmtimer,LED1_PWMchannel,0);
				if(rand_value==1) PWM_SetDutyAdapt(LED2_pwmtimer,LED2_PWMchannel,(uint32_t)Current_Value[1]); else PWM_SetDutyAdapt(LED2_pwmtimer,LED2_PWMchannel,0);
				if(rand_value==2) PWM_SetDutyAdapt(LED3_pwmtimer,LED3_PWMchannel,(uint32_t)Current_Value[2]); else PWM_SetDutyAdapt(LED3_pwmtimer,LED3_PWMchannel,0);
				if(rand_value==3) PWM_SetDutyAdapt(LED4_pwmtimer,LED4_PWMchannel,(uint32_t)Current_Value[3]); else PWM_SetDutyAdapt(LED4_pwmtimer,LED4_PWMchannel,0);
				if(rand_value==4) PWM_SetDutyAdapt(LED5_pwmtimer,LED5_PWMchannel,(uint32_t)Current_Value[4]); else PWM_SetDutyAdapt(LED5_pwmtimer,LED5_PWMchannel,0);
				if(rand_value==5) PWM_SetDutyAdapt(LED6_pwmtimer,LED6_PWMchannel,(uint32_t)Current_Value[5]); else PWM_SetDutyAdapt(LED6_pwmtimer,LED6_PWMchannel,0);
				if(rand_value==6) PWM_SetDutyAdapt(LED7_pwmtimer,LED7_PWMchannel,(uint32_t)Current_Value[6]); else PWM_SetDutyAdapt(LED7_pwmtimer,LED7_PWMchannel,0);
				if(rand_value==7) PWM_SetDutyAdapt(LED8_pwmtimer,LED8_PWMchannel,(uint32_t)Current_Value[7]); else PWM_SetDutyAdapt(LED8_pwmtimer,LED8_PWMchannel,0);
				if(rand_value==8) PWM_SetDutyAdapt(LED9_pwmtimer,LED9_PWMchannel,(uint32_t)Current_Value[8]); else PWM_SetDutyAdapt(LED9_pwmtimer,LED9_PWMchannel,0);
				if(rand_value==9) PWM_SetDutyAdapt(LED10_pwmtimer,LED10_PWMchannel,(uint32_t)Current_Value[9]); else PWM_SetDutyAdapt(LED10_pwmtimer,LED10_PWMchannel,0);

				cpt_strob=0;
			}
			else
			{
				PWM_SetDutyAdapt(LED1_pwmtimer,LED1_PWMchannel,0);
				PWM_SetDutyAdapt(LED2_pwmtimer,LED2_PWMchannel,0);
				PWM_SetDutyAdapt(LED3_pwmtimer,LED3_PWMchannel,0);
				PWM_SetDutyAdapt(LED4_pwmtimer,LED4_PWMchannel,0);
				PWM_SetDutyAdapt(LED5_pwmtimer,LED5_PWMchannel,0);
				PWM_SetDutyAdapt(LED6_pwmtimer,LED6_PWMchannel,0);
				PWM_SetDutyAdapt(LED7_pwmtimer,LED7_PWMchannel,0);
				PWM_SetDutyAdapt(LED8_pwmtimer,LED8_PWMchannel,0);
				PWM_SetDutyAdapt(LED9_pwmtimer,LED9_PWMchannel,0);
				PWM_SetDutyAdapt(LED10_pwmtimer,LED10_PWMchannel,0);

				cpt_strob++;
			}
		}
		else	//NON STROB
		{
			PWM_SetDutyAdapt(LED1_pwmtimer,LED1_PWMchannel,(uint32_t)Current_Value[0]);
			PWM_SetDutyAdapt(LED2_pwmtimer,LED2_PWMchannel,(uint32_t)Current_Value[1]);
			PWM_SetDutyAdapt(LED3_pwmtimer,LED3_PWMchannel,(uint32_t)Current_Value[2]);
			PWM_SetDutyAdapt(LED4_pwmtimer,LED4_PWMchannel,(uint32_t)Current_Value[3]);
			PWM_SetDutyAdapt(LED5_pwmtimer,LED5_PWMchannel,(uint32_t)Current_Value[4]);
			PWM_SetDutyAdapt(LED6_pwmtimer,LED6_PWMchannel,(uint32_t)Current_Value[5]);
			PWM_SetDutyAdapt(LED7_pwmtimer,LED7_PWMchannel,(uint32_t)Current_Value[6]);
			PWM_SetDutyAdapt(LED8_pwmtimer,LED8_PWMchannel,(uint32_t)Current_Value[7]);
			PWM_SetDutyAdapt(LED9_pwmtimer,LED9_PWMchannel,(uint32_t)Current_Value[8]);
			PWM_SetDutyAdapt(LED10_pwmtimer,LED10_PWMchannel,(uint32_t)Current_Value[9]);
			cpt_strob=0;
		}

	}
}

void AppIHMTask()
{
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
}

/* Public function -----------------------------------------------*/
void App_Init()
{
	Load_Param();
	Display_Cursor = 0;

	PatchPWMtoLED(IsInverted);

	switch(Current_Mode)
	{
		case MODE_OFF:
			Current_Display = DISP_CONFIG_MODE;
			break;
		case MODE_MANU:
			Current_Display = DISP_CONFIG_MANVALUE;
			break;
		default:
			Current_Display = DISP_CONFIG_ADDRESS;
			break;
	}


	DMX_signal_OK = __FALSE;

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

	GENE_I2C_Init(GPIOB, SDA_IO_Pin, GPIOB, SCL_IO_Pin);
	Protocol_DMX_init(DMX_Adress,DMX_uart);
	//SSD1306_Init(hi2c_display);  // initialise
	SSD1306_Init();  // initialise

	Bp_Up = BP_OFF;
	Bp_Down = BP_OFF;
	Bp_Ok = BP_OFF;

	HAL_TIM_Base_Start_IT(Tick_Timer);
}

void ManageFan()
{
	static uint32_t timeRefreshFan=0;
	static uint32_t timePWMFan=0;
	static uint8_t FAN_Duty;
	static uint32_t FAN_Mean;
	static uint8_t Cpt_Mean;

	//update FAN
	if(HAL_GetTick() > timeRefreshFan+REFRESH_FAN)
	{
		if(Cpt_Mean>0)
		{
			FAN_Mean += Mean_Value;
			Cpt_Mean--;
		}
		else
		{
			Cpt_Mean=100;
			FAN_Duty = FAN_Mean/100;
			FAN_Mean=0;
		}
		timeRefreshFan = HAL_GetTick();
	}

	if(FAN_Duty==0)											//power  0%
		HAL_GPIO_WritePin(FAN_GPIO_Port, FAN_Pin, GPIO_PIN_RESET);
	else if(FAN_Duty>128)									//power > 50%
		HAL_GPIO_WritePin(FAN_GPIO_Port, FAN_Pin, GPIO_PIN_SET);
	else if(FAN_Duty>0 && HAL_GetTick()>timePWMFan+200)	//power > 0%
	{
		HAL_GPIO_TogglePin(FAN_GPIO_Port, FAN_Pin);
		timePWMFan = HAL_GetTick();
	}
}

void App()
{
	static uint32_t timeRefreshDisplay=0;


	//Update Display
	if(HAL_GetTick() > timeRefreshDisplay+REFRESH_DISPLAY)
	{
		timeRefreshDisplay = HAL_GetTick();
		Update_Display();
		HAL_GPIO_TogglePin(GPIOC, LED_V_Pin);
	}

	//Save Param if change
	if(HAL_GetTick()>(tick_save_param+DELAY_SAVE_PARAM) && param_changed==__TRUE)
	{
		param_changed = __FALSE;
		Write_Param();
		PatchPWMtoLED(IsInverted);
	}

}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
		AppLEDTask();
		AppIHMTask();
		ManageFan();
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	Protocol_DMX_UartCallback(huart);
}
