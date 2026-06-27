################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/CONTROL/auto_control.c \
../Core/Src/CONTROL/state.c \
../Core/Src/CONTROL/transmit.c 

OBJS += \
./Core/Src/CONTROL/auto_control.o \
./Core/Src/CONTROL/state.o \
./Core/Src/CONTROL/transmit.o 

C_DEPS += \
./Core/Src/CONTROL/auto_control.d \
./Core/Src/CONTROL/state.d \
./Core/Src/CONTROL/transmit.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/CONTROL/%.o Core/Src/CONTROL/%.su Core/Src/CONTROL/%.cyclo: ../Core/Src/CONTROL/%.c Core/Src/CONTROL/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H533xx -c -I../Core/Inc -I../Drivers/CMSIS/Include -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32H5xx_HAL_Driver/Inc -I"/home/yugo/NUCLEO_FCCM/Core/Inc/CMD" -I"/home/yugo/NUCLEO_FCCM/Core/Inc/COMUNICATION_PROTOCALS" -I"/home/yugo/NUCLEO_FCCM/Core/Inc/CONTROL" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-CONTROL

clean-Core-2f-Src-2f-CONTROL:
	-$(RM) ./Core/Src/CONTROL/auto_control.cyclo ./Core/Src/CONTROL/auto_control.d ./Core/Src/CONTROL/auto_control.o ./Core/Src/CONTROL/auto_control.su ./Core/Src/CONTROL/state.cyclo ./Core/Src/CONTROL/state.d ./Core/Src/CONTROL/state.o ./Core/Src/CONTROL/state.su ./Core/Src/CONTROL/transmit.cyclo ./Core/Src/CONTROL/transmit.d ./Core/Src/CONTROL/transmit.o ./Core/Src/CONTROL/transmit.su

.PHONY: clean-Core-2f-Src-2f-CONTROL

