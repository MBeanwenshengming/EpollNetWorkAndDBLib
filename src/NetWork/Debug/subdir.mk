################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../CRC32.cpp \
../LAcceptThread.cpp \
../LAutoReleaseMutex.cpp \
../LCloseSocketThread.cpp \
../LConnector.cpp \
../LEpollThread.cpp \
../LEpollThreadManager.cpp \
../LFixLenCircleBuf.cpp \
../LIniFileReadAndWrite.cpp \
../LListenSocket.cpp \
../LNetWorkConfigFileProcessor.cpp \
../LNetWorkServices.cpp \
../LPacketBase.cpp \
../LPacketPoolManager.cpp \
../LPacketSingle.cpp \
../LRecvThread.cpp \
../LRecvThreadManager.cpp \
../LSendThread.cpp \
../LSendThreadManager.cpp \
../LServerBaseNetWork.cpp \
../LSession.cpp \
../LSessionManager.cpp \
../LSocket.cpp \
../LThreadBase.cpp \
../LVarLenCircleBuf.cpp 

OBJS += \
./CRC32.o \
./LAcceptThread.o \
./LAutoReleaseMutex.o \
./LCloseSocketThread.o \
./LConnector.o \
./LEpollThread.o \
./LEpollThreadManager.o \
./LFixLenCircleBuf.o \
./LIniFileReadAndWrite.o \
./LListenSocket.o \
./LNetWorkConfigFileProcessor.o \
./LNetWorkServices.o \
./LPacketBase.o \
./LPacketPoolManager.o \
./LPacketSingle.o \
./LRecvThread.o \
./LRecvThreadManager.o \
./LSendThread.o \
./LSendThreadManager.o \
./LServerBaseNetWork.o \
./LSession.o \
./LSessionManager.o \
./LSocket.o \
./LThreadBase.o \
./LVarLenCircleBuf.o 

CPP_DEPS += \
./CRC32.d \
./LAcceptThread.d \
./LAutoReleaseMutex.d \
./LCloseSocketThread.d \
./LConnector.d \
./LEpollThread.d \
./LEpollThreadManager.d \
./LFixLenCircleBuf.d \
./LIniFileReadAndWrite.d \
./LListenSocket.d \
./LNetWorkConfigFileProcessor.d \
./LNetWorkServices.d \
./LPacketBase.d \
./LPacketPoolManager.d \
./LPacketSingle.d \
./LRecvThread.d \
./LRecvThreadManager.d \
./LSendThread.d \
./LSendThreadManager.d \
./LServerBaseNetWork.d \
./LSession.d \
./LSessionManager.d \
./LSocket.d \
./LThreadBase.d \
./LVarLenCircleBuf.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


