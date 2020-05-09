/**
  ******************************************************************************
  * @file           : App.h
  * @brief          : Application
  ******************************************************************************
  * @attention
  * - works with FreeRTOS
  * - use dmx callback in uart IT
  ******************************************************************************
  */

/* Public prototypes -----------------------------------------------*/
void App_Init(UART_HandleTypeDef* ref_uart,TIM_HandleTypeDef* LED_pwmtimer, uint32_t LED_PWMchannel, I2C_HandleTypeDef* hi2c_display);
void CreatAppTasks (void);
