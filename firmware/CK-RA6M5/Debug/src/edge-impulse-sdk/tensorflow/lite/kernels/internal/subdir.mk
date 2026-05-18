################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/edge-impulse-sdk/tensorflow/lite/kernels/internal/portable_tensor_utils.cc \
../src/edge-impulse-sdk/tensorflow/lite/kernels/internal/quantization_util.cc \
../src/edge-impulse-sdk/tensorflow/lite/kernels/internal/reference_portable_tensor_utils.cc \
../src/edge-impulse-sdk/tensorflow/lite/kernels/internal/tensor_utils.cc 

SREC += \
assssss.srec 

CC_DEPS += \
./src/edge-impulse-sdk/tensorflow/lite/kernels/internal/portable_tensor_utils.d \
./src/edge-impulse-sdk/tensorflow/lite/kernels/internal/quantization_util.d \
./src/edge-impulse-sdk/tensorflow/lite/kernels/internal/reference_portable_tensor_utils.d \
./src/edge-impulse-sdk/tensorflow/lite/kernels/internal/tensor_utils.d 

OBJS += \
./src/edge-impulse-sdk/tensorflow/lite/kernels/internal/portable_tensor_utils.o \
./src/edge-impulse-sdk/tensorflow/lite/kernels/internal/quantization_util.o \
./src/edge-impulse-sdk/tensorflow/lite/kernels/internal/reference_portable_tensor_utils.o \
./src/edge-impulse-sdk/tensorflow/lite/kernels/internal/tensor_utils.o 

MAP += \
assssss.map 


# Each subdirectory must supply rules for building sources it contributes
src/edge-impulse-sdk/tensorflow/lite/kernels/internal/%.o: ../src/edge-impulse-sdk/tensorflow/lite/kernels/internal/%.cc
	$(file > $@.in,-mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -O2 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-strict-aliasing -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal -g -D_RA_CORE=CM33 -D_RA_ORDINAL=1 -I"C:/workspace2/assssss/ra_gen" -I"C:/workspace2/assssss/src/tflite-model" -I"C:/workspace2/assssss/src/model-parameters" -I"C:/workspace2/assssss/src/edge-impulse-sdk" -I"C:/workspace2/assssss/src" -I"." -I"C:/workspace2/assssss/ra_cfg/fsp_cfg/bsp" -I"C:/workspace2/assssss/ra_cfg/fsp_cfg" -I"C:/workspace2/assssss/src" -I"C:/workspace2/assssss/ra/fsp/inc" -I"C:/workspace2/assssss/ra/fsp/inc/api" -I"C:/workspace2/assssss/ra/fsp/inc/instances" -I"C:/workspace2/assssss/ra/arm/CMSIS_6/CMSIS/Core/Include" -std=c++11 -fabi-version=0 -Wno-stringop-overflow -Wno-format-truncation --param=min-pagesize=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" -x c++ "$<")
	@echo Building file: $< && arm-none-eabi-g++ @"$@.in"

