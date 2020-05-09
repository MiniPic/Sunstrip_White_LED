################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/Devices/PWM/PWM.c 

OBJS += \
./Drivers/Devices/PWM/PWM.o 

C_DEPS += \
./Drivers/Devices/PWM/PWM.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/Devices/PWM/PWM.o: ../Drivers/Devices/PWM/PWM.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32F103xB -DDEBUG -c -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3 -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Drivers/Devices/PWM/PWM.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

