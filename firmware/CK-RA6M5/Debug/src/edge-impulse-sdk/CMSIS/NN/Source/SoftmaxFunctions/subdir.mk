################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/edge-impulse-sdk/CMSIS/NN/Source/SoftmaxFunctions/arm_nn_softmax_common_s8.c \
../src/edge-impulse-sdk/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_q15.c \
../src/edge-impulse-sdk/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_q7.c \
../src/edge-impulse-sdk/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_s16.c \
../src/edge-impulse-sdk/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_s8.c \
../src/edge-impulse-sdk/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_s8_s16.c \
../src/edge-impulse-sdk/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_u8.c \
../src/edge-impulse-sdk/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_with_batch_q7.c 

SREC += \
assssss.srec 

C_DEPS += \
./src/edge-impulse-sdk/CMSIS/NN/Source/SoftmaxFunctions/arm_nn_softmax_common_s8.d \
./src/edge-impulse-sdk/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_q15.d \
./src/edge-impulse-sdk/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_q7.d \
./src/edge-impulse-sdk/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_s16.d \
./src/edge-impulse-sdk/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_s8.d \
./src/edge-impulse-sdk/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_s8_s16.d \
./src/edge-impulse-sdk/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_u8.d \
./src/edge-impulse-sdk/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_with_batch_q7.d 

OBJS += \
./src/edge-impulse-sdk/CMSIS/NN/Source/SoftmaxFunctions/arm_nn_softmax_common_s8.o \
./src/edge-impulse-sdk/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_q15.o \
./src/edge-impulse-sdk/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_q7.o \
./src/edge-impulse-sdk/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_s16.o \
./src/edge-impulse-sdk/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_s8.o \
./src/edge-impulse-sdk/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_s8_s16.o \
./src/edge-impulse-sdk/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_u8.o \
./src/edge-impulse-sdk/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_with_batch_q7.o 

MAP += \
assssss.map 


# Each subdirectory must supply rules for building sources it contributes
src/edge-impulse-sdk/CMSIS/NN/Source/SoftmaxFunctions/%.o: ../src/edge-impulse-sdk/CMSIS/NN/Source/SoftmaxFunctions/%.c
	$(file > $@.in,-mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -O2 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-strict-aliasing -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal -g -D_RENESAS_RA_ -D_RA_CORE=CM33 -D_RA_ORDINAL=1 -I"C:/workspace2/assssss/ra_gen" -I"." -I"C:/workspace2/assssss/ra_cfg/fsp_cfg/bsp" -I"C:/workspace2/assssss/ra_cfg/fsp_cfg" -I"C:/workspace2/assssss/src" -I"C:/workspace2/assssss/ra/fsp/inc" -I"C:/workspace2/assssss/ra/fsp/inc/api" -I"C:/workspace2/assssss/ra/fsp/inc/instances" -I"C:/workspace2/assssss/ra/arm/CMSIS_6/CMSIS/Core/Include" -std=c99 -Wno-stringop-overflow -Wno-format-truncation --param=min-pagesize=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" -x c "$<")
	@echo Building file: $< && arm-none-eabi-gcc @"$@.in"

