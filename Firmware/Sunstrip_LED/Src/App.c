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

#define		TASK_DELAY_LED				50
#define		TASK_DELAY_IHM				50
#define		REFRESH_DISPLAY				5				//5*TASK_DELAY_IHM

#define 	DELAY_SAVE_PARAM			5000			//in ms

#define		TIME_LONG_BP				500

#define		__TRUE						0x01
#define 	__FALSE						0x00

#define 	MIN_LED_PWM_VALUE			63
#define 	MAX_LED_PWM_VALUE			244


/* Types Definitions ---------------------------------------------------------*/
typedef enum {	MODE_OFF = 0x00,
				MODE_DMX = 0x01,
				MODE_MANU = 0x02
}App_Mode;

typedef enum {	BP_OFF,
				BP_CLICK,
				BP_1s,
				BP_IDLE,
}BP_Status;

/* Private variables ---------------------------------------------------------*/
osThreadId 			AppLEDTaskHandle;
osThreadId 			AppIHMTaskHandle;

uint16_t			DMX_Adress;

uint8_t				DMX_values[10];
uint8_t				Manu_value;
uint8_t				DMX_signal_OK;
App_Mode			Current_Mode;

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
		Current_Mode = 	(val_param&0xFF000000)>>24;
		Manu_value = 	(val_param&0x00FF0000)>>16;
		DMX_Adress = 	(val_param&0x0000FFFF);
		return __TRUE;
	}
	else
	{
		Current_Mode = MODE_OFF;
		Manu_value = 100;
		DMX_Adress = 1;
		return __FALSE;
	}
}

uint8_t Write_Param()
{
	uint32_t data[2];
	data[0] = PARAM_EXIST_CODE;
	data[1] = 0x00;
	data[1] |= (Current_Mode<<24);
	data[1] |= (Manu_value<<16);
	data[1] |= (DMX_Adress);

	FlashManager_WriteMulti(PARAM_EXIST_ADDRESS,2,data);
}

void Update_Display()
{
	char Str_percent[]="000";
    char Str_dmx[]="000";
    char Str_add[]="000";
    uint8_t percent_value;

    SSD1306_Clear();

    if(Current_Mode == MODE_OFF)			//OFF
    {
    	SSD1306_GotoXY (40,0);
    	SSD1306_Puts ("OFF", &Font_16x26, 1);
    }
    else if(Current_Mode == MODE_MANU)		//MANU
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
    else								//DMX
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
	for(;;)
	{
		osDelay(TASK_DELAY_LED);

		if(Current_Mode == MODE_OFF)
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
			Update_Display();
			cpt_refresh=0;
		}

		Manage_Button();

		if(Bp_Ok == BP_CLICK)
		{
			if(Current_Mode==MODE_OFF)
				Current_Mode=MODE_MANU;
			else if(Current_Mode==MODE_MANU)
				Current_Mode=MODE_DMX;
			else
				Current_Mode=MODE_OFF;

			param_changed = __TRUE;
			tick_save_param = HAL_GetTick();
		}

		if(Current_Mode==MODE_MANU)
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
		}

		if(Current_Mode==MODE_DMX)
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
		}

		if(HAL_GetTick()>tick_save_param+DELAY_SAVE_PARAM && param_changed==__TRUE)
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

	DMX_signal_OK = __FALSE;

	PWM_SetPWM(LED1_pwmtimer,LED1_PWMchannel,LED_PWM_PERIOD_VALUE,0);		//PWM Off
	PWM_SetPWM(LED2_pwmtimer,LED2_PWMchannel,LED_PWM_PERIOD_VALUE,0);		//PWM Off
	PWM_SetPWM(LED3_pwmtimer,LED3_PWMchannel,LED_PWM_PERIOD_VALUE,0);		//PWM Off
	PWM_SetPWM(LED4_pwmtimer,LED4_PWMchannel,LED_PWM_PERIOD_VALUE,0);		//PWM Off
	PWM_SetPWM(LED5_pwmtimer,LED5_PWMchannel,LED_PWM_PERIOD_VALUE,0);		//PWM Off
	PWM_SetPWM(LED6_pwmtimer,LED6_PWMchannel,LED_PWM_PERIOD_VALUE,0);		//PWM Off
	PWM_SetPWM(LED7_pwmtimer,LED7_PWMchannel,LED_PWM_PERIOD_VALUE,0);		//PWM Off
	PWM_SetPWM(LED8_pwmtimer,LED8_PWMchannel,LED_PWM_PERIOD_VALUE,0);		//PWM Off
	PWM_SetPWM(LED9_pwmtimer,LED9_PWMchannel,LED_PWM_PERIOD_VALUE,0);		//PWM Off
	PWM_SetPWM(LED10_pwmtimer,LED10_PWMchannel,LED_PWM_PERIOD_VALUE,0);		//PWM Off

	Protocol_DMX_init(DMX_Adress,DMX_uart);
	SSD1306_Init(hi2c_display);  // initialise

	Bp_Up = BP_OFF;
	Bp_Down = BP_OFF;
	Bp_Ok = BP_OFF;

	CreatAppTasks();
}

void CreatAppTasks (void)
{
	osThreadDef(App_LED_Task, AppLEDTask, osPriorityHigh, 0, 128);
	AppLEDTaskHandle = osThreadCreate(osThread(App_LED_Task), NULL);

	osThreadDef(App_IHM_Task, AppIHMTask, osPriorityNormal, 0, 256);
	AppIHMTaskHandle = osThreadCreate(osThread(App_IHM_Task), NULL);
}


HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	Protocol_DMX_UartCallback(huart);

	return 0;
}
