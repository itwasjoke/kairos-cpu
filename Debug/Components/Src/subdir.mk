################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Components/Src/CAN.c \
../Components/Src/Config.c \
../Components/Src/Ethernet.c \
../Components/Src/RS485.c \
../Components/Src/eeprom.c \
../Components/Src/i2c.c \
../Components/Src/kairos.c \
../Components/Src/led.c 

OBJS += \
./Components/Src/CAN.o \
./Components/Src/Config.o \
./Components/Src/Ethernet.o \
./Components/Src/RS485.o \
./Components/Src/eeprom.o \
./Components/Src/i2c.o \
./Components/Src/kairos.o \
./Components/Src/led.o 

C_DEPS += \
./Components/Src/CAN.d \
./Components/Src/Config.d \
./Components/Src/Ethernet.d \
./Components/Src/RS485.d \
./Components/Src/eeprom.d \
./Components/Src/i2c.d \
./Components/Src/kairos.d \
./Components/Src/led.d 


# Each subdirectory must supply rules for building sources it contributes
Components/Src/%.o Components/Src/%.su Components/Src/%.cyclo: ../Components/Src/%.c Components/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Components-2f-Src

clean-Components-2f-Src:
	-$(RM) ./Components/Src/CAN.cyclo ./Components/Src/CAN.d ./Components/Src/CAN.o ./Components/Src/CAN.su ./Components/Src/Config.cyclo ./Components/Src/Config.d ./Components/Src/Config.o ./Components/Src/Config.su ./Components/Src/Ethernet.cyclo ./Components/Src/Ethernet.d ./Components/Src/Ethernet.o ./Components/Src/Ethernet.su ./Components/Src/RS485.cyclo ./Components/Src/RS485.d ./Components/Src/RS485.o ./Components/Src/RS485.su ./Components/Src/eeprom.cyclo ./Components/Src/eeprom.d ./Components/Src/eeprom.o ./Components/Src/eeprom.su ./Components/Src/i2c.cyclo ./Components/Src/i2c.d ./Components/Src/i2c.o ./Components/Src/i2c.su ./Components/Src/kairos.cyclo ./Components/Src/kairos.d ./Components/Src/kairos.o ./Components/Src/kairos.su ./Components/Src/led.cyclo ./Components/Src/led.d ./Components/Src/led.o ./Components/Src/led.su

.PHONY: clean-Components-2f-Src

