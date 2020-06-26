/**
  ******************************************************************************
  * @file           : App.h
  * @brief          : Application
  ******************************************************************************
  * @attention
  * - works with FreeRTOS
  * - use dmx callback in uart IT
  * - 10 channel for LED dimmer
  * - 1 channel for strob (0-128 total; 129-255 random)
  * - 1 channel for speed (0 to 1020ms)
  ******************************************************************************
  */

/* Public variables ----------------------------------------------------------*/
UART_HandleTypeDef* DMX_uart;

TIM_HandleTypeDef* 	LED1_pwmtimer;
uint32_t 			LED1_PWMchannel;
TIM_HandleTypeDef* 	LED2_pwmtimer;
uint32_t 			LED2_PWMchannel;
TIM_HandleTypeDef* 	LED3_pwmtimer;
uint32_t 			LED3_PWMchannel;
TIM_HandleTypeDef* 	LED4_pwmtimer;
uint32_t 			LED4_PWMchannel;
TIM_HandleTypeDef* 	LED5_pwmtimer;
uint32_t 			LED5_PWMchannel;
TIM_HandleTypeDef* 	LED6_pwmtimer;
uint32_t 			LED6_PWMchannel;
TIM_HandleTypeDef* 	LED7_pwmtimer;
uint32_t 			LED7_PWMchannel;
TIM_HandleTypeDef* 	LED8_pwmtimer;
uint32_t 			LED8_PWMchannel;
TIM_HandleTypeDef* 	LED9_pwmtimer;
uint32_t 			LED9_PWMchannel;
TIM_HandleTypeDef* 	LED10_pwmtimer;
uint32_t 			LED10_PWMchannel;

TIM_HandleTypeDef* 	FAN_pwmtimer;
uint32_t 			FAN_PWMchannel;


//I2C_HandleTypeDef* 	hi2c_display;

/* Public prototypes -----------------------------------------------*/
void App_Init();
void CreatAppTasks (void);
