################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/System/Flash_Manager.c 

OBJS += \
./Drivers/System/Flash_Manager.o 

C_DEPS += \
./Drivers/System/Flash_Manager.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/System/Flash_Manager.o: ../Drivers/System/Flash_Manager.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -O3 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Drivers/System/Flash_Manager.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

