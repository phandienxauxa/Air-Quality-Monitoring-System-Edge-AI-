################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/ML_model/edge-impulse-sdk/tensorflow/lite/core/api/common.cc \
../src/ML_model/edge-impulse-sdk/tensorflow/lite/core/api/error_reporter.cc \
../src/ML_model/edge-impulse-sdk/tensorflow/lite/core/api/flatbuffer_conversions.cc \
../src/ML_model/edge-impulse-sdk/tensorflow/lite/core/api/op_resolver.cc \
../src/ML_model/edge-impulse-sdk/tensorflow/lite/core/api/tensor_utils.cc 

SREC += \
CK-RA6M5.srec 

CC_DEPS += \
./src/ML_model/edge-impulse-sdk/tensorflow/lite/core/api/common.d \
./src/ML_model/edge-impulse-sdk/tensorflow/lite/core/api/error_reporter.d \
./src/ML_model/edge-impulse-sdk/tensorflow/lite/core/api/flatbuffer_conversions.d \
./src/ML_model/edge-impulse-sdk/tensorflow/lite/core/api/op_resolver.d \
./src/ML_model/edge-impulse-sdk/tensorflow/lite/core/api/tensor_utils.d 

OBJS += \
./src/ML_model/edge-impulse-sdk/tensorflow/lite/core/api/common.o \
./src/ML_model/edge-impulse-sdk/tensorflow/lite/core/api/error_reporter.o \
./src/ML_model/edge-impulse-sdk/tensorflow/lite/core/api/flatbuffer_conversions.o \
./src/ML_model/edge-impulse-sdk/tensorflow/lite/core/api/op_resolver.o \
./src/ML_model/edge-impulse-sdk/tensorflow/lite/core/api/tensor_utils.o 

MAP += \
CK-RA6M5.map 


# Each subdirectory must supply rules for building sources it contributes
src/ML_model/edge-impulse-sdk/tensorflow/lite/core/api/%.o: ../src/ML_model/edge-impulse-sdk/tensorflow/lite/core/api/%.cc
	$(file > $@.in,-mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -O2 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-strict-aliasing -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal -g -D_RA_CORE=CM33 -D_RA_ORDINAL=1 -I"C:/Users/admin/Downloads/CK-RA6M5/ra_gen" -I"C:/Users/admin/Downloads/CK-RA6M5/src/Include" -I"C:/Users/admin/Downloads/CK-RA6M5/src/ML_model" -I"C:/workspace2/assssss/src" -I"." -I"C:/Users/admin/Downloads/CK-RA6M5/ra_cfg/fsp_cfg/bsp" -I"C:/Users/admin/Downloads/CK-RA6M5/ra_cfg/fsp_cfg" -I"C:/Users/admin/Downloads/CK-RA6M5/src" -I"C:/Users/admin/Downloads/CK-RA6M5/ra/fsp/inc" -I"C:/Users/admin/Downloads/CK-RA6M5/ra/fsp/inc/api" -I"C:/Users/admin/Downloads/CK-RA6M5/ra/fsp/inc/instances" -I"C:/Users/admin/Downloads/CK-RA6M5/ra/arm/CMSIS_6/CMSIS/Core/Include" -std=c++11 -fabi-version=0 -Wno-stringop-overflow -Wno-format-truncation --param=min-pagesize=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" -x c++ "$<")
	@echo Building file: $< && arm-none-eabi-g++ @"$@.in"

