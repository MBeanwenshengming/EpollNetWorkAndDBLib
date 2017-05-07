################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../LMainLogicBroadCast.cpp \
../LMainTest.cpp \
../LMyErrorRecorder.cpp 

OBJS += \
./LMainLogicBroadCast.o \
./LMainTest.o \
./LMyErrorRecorder.o 

CPP_DEPS += \
./LMainLogicBroadCast.d \
./LMainTest.d \
./LMyErrorRecorder.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/home/wenshengming/work/EpollNetWork/NetWork -I"/home/wenshengming/work/EpollNetWork/singleserver/singleserver/packetdefine" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


