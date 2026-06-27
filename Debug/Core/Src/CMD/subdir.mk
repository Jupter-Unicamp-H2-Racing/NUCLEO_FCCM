################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/CMD/contactor.c \
../Core/Src/CMD/fan.c \
../Core/Src/CMD/valve.c 

OBJS += \
./Core/Src/CMD/contactor.o \
./Core/Src/CMD/fan.o \
./Core/Src/CMD/valve.o 

C_DEPS += \
./Core/Src/CMD/contactor.d \
./Core/Src/CMD/fan.d \
./Core/Src/CMD/valve.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/CMD/%.o Core/Src/CMD/%.su Core/Src/CMD/%.cyclo: ../Core/Src/CMD/%.c Core/Src/CMD/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H533xx -c -I../Core/Inc -I../Drivers/CMSIS/Include -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32H5xx_HAL_Driver/Inc -I"/home/yugo/NUCLEO_FCCM/Core/Inc/CMD" -I"/home/yugo/NUCLEO_FCCM/Core/Inc/COMUNICATION_PROTOCALS" -I"/home/yugo/NUCLEO_FCCM/Core/Inc/CONTROL" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-CMD

clean-Core-2f-Src-2f-CMD:
	-$(RM) ./Core/Src/CMD/contactor.cyclo ./Core/Src/CMD/contactor.d ./Core/Src/CMD/contactor.o ./Core/Src/CMD/contactor.su ./Core/Src/CMD/fan.cyclo ./Core/Src/CMD/fan.d ./Core/Src/CMD/fan.o ./Core/Src/CMD/fan.su ./Core/Src/CMD/valve.cyclo ./Core/Src/CMD/valve.d ./Core/Src/CMD/valve.o ./Core/Src/CMD/valve.su

.PHONY: clean-Core-2f-Src-2f-CMD

