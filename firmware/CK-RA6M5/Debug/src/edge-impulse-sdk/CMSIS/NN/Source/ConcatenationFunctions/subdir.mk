################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/edge-impulse-sdk/CMSIS/NN/Source/ConcatenationFunctions/arm_concatenation_s8_w.c \
../src/edge-impulse-sdk/CMSIS/NN/Source/ConcatenationFunctions/arm_concatenation_s8_x.c \
../src/edge-impulse-sdk/CMSIS/NN/Source/ConcatenationFunctions/arm_concatenation_s8_y.c \
../src/edge-impulse-sdk/CMSIS/NN/Source/ConcatenationFunctions/arm_concatenation_s8_z.c 

SREC += \
assssss.srec 

C_DEPS += \
./src/edge-impulse-sdk/CMSIS/NN/Source/ConcatenationFunctions/arm_concatenation_s8_w.d \
./src/edge-impulse-sdk/CMSIS/NN/Source/ConcatenationFunctions/arm_concatenation_s8_x.d \
./src/edge-impulse-sdk/CMSIS/NN/Source/ConcatenationFunctions/arm_concatenation_s8_y.d \
./src/edge-impulse-sdk/CMSIS/NN/Source/ConcatenationFunctions/arm_concatenation_s8_z.d 

OBJS += \
./src/edge-impulse-sdk/CMSIS/NN/Source/ConcatenationFunctions/arm_concatenation_s8_w.o \
./src/edge-impulse-sdk/CMSIS/NN/Source/ConcatenationFunctions/arm_concatenation_s8_x.o \
./src/edge-impulse-sdk/CMSIS/NN/Source/ConcatenationFunctions/arm_concatenation_s8_y.o \
./src/edge-impulse-sdk/CMSIS/NN/Source/ConcatenationFunctions/arm_concatenation_s8_z.o 

MAP += \
assssss.map 


# Each subdirectory must supply rules for building sources it contributes
src/edge-impulse-sdk/CMSIS/NN/Source/ConcatenationFunctions/%.o: ../src/edge-impulse-sdk/CMSIS/NN/Source/ConcatenationFunctions/%.c
	$(file > $@.in,-mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -O2 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-strict-aliasing -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal -g -D_RENESAS_RA_ -D_RA_CORE=CM33 -D_RA_ORDINAL=1 -I"C:/workspace2/assssss/ra_gen" -I"." -I"C:/workspace2/assssss/ra_cfg/fsp_cfg/bsp" -I"C:/workspace2/assssss/ra_cfg/fsp_cfg" -I"C:/workspace2/assssss/src" -I"C:/workspace2/assssss/ra/fsp/inc" -I"C:/workspace2/assssss/ra/fsp/inc/api" -I"C:/workspace2/assssss/ra/fsp/inc/instances" -I"C:/workspace2/assssss/ra/arm/CMSIS_6/CMSIS/Core/Include" -std=c99 -Wno-stringop-overflow -Wno-format-truncation --param=min-pagesize=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" -x c "$<")
	@echo Building file: $< && arm-none-eabi-gcc @"$@.in"

