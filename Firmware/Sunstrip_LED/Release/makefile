################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Startup/subdir.mk
-include Src/subdir.mk
-include Drivers/System/subdir.mk
-include Drivers/STM32F1xx_HAL_Driver/Src/subdir.mk
-include Drivers/Devices/PWM/subdir.mk
-include Drivers/Devices/Display/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
Sunstrip_LED.elf \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
Sunstrip_LED.list \

OBJCOPY_BIN += \
Sunstrip_LED.bin \


# All Target
all: Sunstrip_LED.elf secondary-outputs

# Tool invocations
Sunstrip_LED.elf: $(OBJS) $(USER_OBJS) Z:\TECHNIQUE\Projets\Sunstrip_White_LED\Firmware\Sunstrip_LED\STM32F103C8TX_FLASH.ld
	arm-none-eabi-gcc -o "Sunstrip_LED.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m3 -T"Z:\TECHNIQUE\Projets\Sunstrip_White_LED\Firmware\Sunstrip_LED\STM32F103C8TX_FLASH.ld" --specs=nosys.specs -Wl,-Map="Sunstrip_LED.map" -Wl,--gc-sections -static --specs=nano.specs -mfloat-abi=soft -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

Sunstrip_LED.list: $(EXECUTABLES)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "Sunstrip_LED.list"
	@echo 'Finished building: $@'
	@echo ' '

Sunstrip_LED.bin: $(EXECUTABLES)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "Sunstrip_LED.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) *
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_BIN)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
