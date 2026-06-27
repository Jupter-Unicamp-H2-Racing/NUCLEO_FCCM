################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/COMUNICATION_PROTOCALS/can.c \
../Core/Src/COMUNICATION_PROTOCALS/uart.c 

OBJS += \
./Core/Src/COMUNICATION_PROTOCALS/can.o \
./Core/Src/COMUNICATION_PROTOCALS/uart.o 

C_DEPS += \
./Core/Src/COMUNICATION_PROTOCALS/can.d \
./Core/Src/COMUNICATION_PROTOCALS/uart.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/COMUNICATION_PROTOCALS/%.o Core/Src/COMUNICATION_PROTOCALS/%.su Core/Src/COMUNICATION_PROTOCALS/%.cyclo: ../Core/Src/COMUNICATION_PROTOCALS/%.c Core/Src/COMUNICATION_PROTOCALS/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H533xx -c -I../Core/Inc -I../Drivers/CMSIS/Include -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32H5xx_HAL_Driver/Inc -I"/home/yugo/NUCLEO_FCCM/Core/Inc/CMD" -I"/home/yugo/NUCLEO_FCCM/Core/Inc/COMUNICATION_PROTOCALS" -I"/home/yugo/NUCLEO_FCCM/Core/Inc/CONTROL" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-COMUNICATION_PROTOCALS

clean-Core-2f-Src-2f-COMUNICATION_PROTOCALS:
	-$(RM) ./Core/Src/COMUNICATION_PROTOCALS/can.cyclo ./Core/Src/COMUNICATION_PROTOCALS/can.d ./Core/Src/COMUNICATION_PROTOCALS/can.o ./Core/Src/COMUNICATION_PROTOCALS/can.su ./Core/Src/COMUNICATION_PROTOCALS/uart.cyclo ./Core/Src/COMUNICATION_PROTOCALS/uart.d ./Core/Src/COMUNICATION_PROTOCALS/uart.o ./Core/Src/COMUNICATION_PROTOCALS/uart.su

.PHONY: clean-Core-2f-Src-2f-COMUNICATION_PROTOCALS

