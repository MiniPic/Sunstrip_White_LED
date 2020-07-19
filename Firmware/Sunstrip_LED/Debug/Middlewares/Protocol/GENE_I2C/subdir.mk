################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middlewares/Protocol/GENE_I2C/GENE_I2C_Master.c 

OBJS += \
./Middlewares/Protocol/GENE_I2C/GENE_I2C_Master.o 

C_DEPS += \
./Middlewares/Protocol/GENE_I2C/GENE_I2C_Master.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/Protocol/GENE_I2C/GENE_I2C_Master.o: ../Middlewares/Protocol/GENE_I2C/GENE_I2C_Master.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32F103xB -DDEBUG -c -I../Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Middlewares/Protocol/GENE_I2C/GENE_I2C_Master.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

