################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../tests/EulerPolesDataTest.cpp \
../tests/PaleoLatitudeTest.cpp \
../tests/PlateDataTest.cpp \
../tests/PolarWanderPathsDataTest.cpp \
../tests/UtilTest.cpp 

OBJS += \
./tests/EulerPolesDataTest.o \
./tests/PaleoLatitudeTest.o \
./tests/PlateDataTest.o \
./tests/PolarWanderPathsDataTest.o \
./tests/UtilTest.o 

CPP_DEPS += \
./tests/EulerPolesDataTest.d \
./tests/PaleoLatitudeTest.d \
./tests/PlateDataTest.d \
./tests/PolarWanderPathsDataTest.d \
./tests/UtilTest.d 


# Each subdirectory must supply rules for building sources it contributes
tests/%.o: ../tests/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


