################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/ML_model/edge-impulse-sdk/tensorflow/lite/kernels/kernel_util_lite.cc 

SREC += \
CK-RA6M5.srec 

CC_DEPS += \
./src/ML_model/edge-impulse-sdk/tensorflow/lite/kernels/kernel_util_lite.d 

OBJS += \
./src/ML_model/edge-impulse-sdk/tensorflow/lite/kernels/kernel_util_lite.o 

MAP += \
CK-RA6M5.map 


# Each subdirectory must supply rules for building sources it contributes
src/ML_model/edge-impulse-sdk/tensorflow/lite/kernels/%.o: ../src/ML_model/edge-impulse-sdk/tensorflow/lite/kernels/%.cc
	$(file > $@.in,-mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -O2 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-strict-aliasing -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal -g -D_RA_CORE=CM33 -D_RA_ORDINAL=1 -I"C:/Users/admin/Downloads/CK-RA6M5/ra_gen" -I"C:/Users/admin/Downloads/CK-RA6M5/src/Include" -I"C:/Users/admin/Downloads/CK-RA6M5/src/ML_model" -I"C:/workspace2/assssss/src" -I"." -I"C:/Users/admin/Downloads/CK-RA6M5/ra_cfg/fsp_cfg/bsp" -I"C:/Users/admin/Downloads/CK-RA6M5/ra_cfg/fsp_cfg" -I"C:/Users/admin/Downloads/CK-RA6M5/src" -I"C:/Users/admin/Downloads/CK-RA6M5/ra/fsp/inc" -I"C:/Users/admin/Downloads/CK-RA6M5/ra/fsp/inc/api" -I"C:/Users/admin/Downloads/CK-RA6M5/ra/fsp/inc/instances" -I"C:/Users/admin/Downloads/CK-RA6M5/ra/arm/CMSIS_6/CMSIS/Core/Include" -std=c++11 -fabi-version=0 -Wno-stringop-overflow -Wno-format-truncation --param=min-pagesize=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" -x c++ "$<")
	@echo Building file: $< && arm-none-eabi-g++ @"$@.in"

