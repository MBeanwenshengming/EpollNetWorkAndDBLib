################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../LConnectorTest.cpp \
../LMainTest.cpp \
../LMyErrorRecorder.cpp 

OBJS += \
./LConnectorTest.o \
./LMainTest.o \
./LMyErrorRecorder.o 

CPP_DEPS += \
./LConnectorTest.d \
./LMainTest.d \
./LMyErrorRecorder.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"/home/wenshengming/work/EpollNetWork/NetWork" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


