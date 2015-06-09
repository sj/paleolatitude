################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/util/Exception.cpp \
../src/util/LogStream.cpp \
../src/util/Logger.cpp \
../src/util/Util.cpp 

OBJS += \
./src/util/Exception.o \
./src/util/LogStream.o \
./src/util/Logger.o \
./src/util/Util.o 

CPP_DEPS += \
./src/util/Exception.d \
./src/util/LogStream.d \
./src/util/Logger.d \
./src/util/Util.d 


# Each subdirectory must supply rules for building sources it contributes
src/util/%.o: ../src/util/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O3 -Wall -c -fmessage-length=0  -std=gnu++11  -DNDEBUG -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


