################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../tests/PlateDataTest.cpp \
../tests/PolarWanderPathsDataTest.cpp 

OBJS += \
./tests/PlateDataTest.o \
./tests/PolarWanderPathsDataTest.o 

CPP_DEPS += \
./tests/PlateDataTest.d \
./tests/PolarWanderPathsDataTest.d 


# Each subdirectory must supply rules for building sources it contributes
tests/%.o: ../tests/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O3 -Wall -c -fmessage-length=0  -std=gnu++0x -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


