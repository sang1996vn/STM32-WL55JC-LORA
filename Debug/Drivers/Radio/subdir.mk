################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/Radio/radio_board_if.c \
../Drivers/Radio/radio_driver.c 

OBJS += \
./Drivers/Radio/radio_board_if.o \
./Drivers/Radio/radio_driver.o 

C_DEPS += \
./Drivers/Radio/radio_board_if.d \
./Drivers/Radio/radio_driver.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/Radio/%.o Drivers/Radio/%.su: ../Drivers/Radio/%.c Drivers/Radio/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DCORE_CM4 -DUSE_HAL_DRIVER -DSTM32WL55xx -c -I../Core/Inc -I../Drivers/STM32WLxx_HAL_Driver/Inc -I../Drivers/STM32WLxx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32WLxx/Include -I../Drivers/CMSIS/Include -I"E:/PARTTIME/Lora/en.stm32cubewl-v1-3-0/STM32Cube_FW_WL_V1.3.0/Projects/NUCLEO-WL55JC/Applications/SubGHz_Phy/Lora_pingpong/Drivers/BSP/STM32WLxx_Nucleo" -I"E:/PARTTIME/Lora/en.stm32cubewl-v1-3-0/STM32Cube_FW_WL_V1.3.0/Projects/NUCLEO-WL55JC/Applications/SubGHz_Phy/Lora_pingpong/Ultilities/conf" -I"E:/PARTTIME/Lora/en.stm32cubewl-v1-3-0/STM32Cube_FW_WL_V1.3.0/Projects/NUCLEO-WL55JC/Applications/SubGHz_Phy/Lora_pingpong/Ultilities/misc" -I"E:/PARTTIME/Lora/en.stm32cubewl-v1-3-0/STM32Cube_FW_WL_V1.3.0/Projects/NUCLEO-WL55JC/Applications/SubGHz_Phy/Lora_pingpong/Drivers/Radio" -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3 -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-Radio

clean-Drivers-2f-Radio:
	-$(RM) ./Drivers/Radio/radio_board_if.d ./Drivers/Radio/radio_board_if.o ./Drivers/Radio/radio_board_if.su ./Drivers/Radio/radio_driver.d ./Drivers/Radio/radio_driver.o ./Drivers/Radio/radio_driver.su

.PHONY: clean-Drivers-2f-Radio

