################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/paleo_latitude/PLEulerPolesReconstructions.cpp \
../src/paleo_latitude/PLParameters.cpp \
../src/paleo_latitude/PLPlate.cpp \
../src/paleo_latitude/PLPlates.cpp \
../src/paleo_latitude/PLPolarWanderPaths.cpp \
../src/paleo_latitude/PaleoLatitude.cpp 

OBJS += \
./src/paleo_latitude/PLEulerPolesReconstructions.o \
./src/paleo_latitude/PLParameters.o \
./src/paleo_latitude/PLPlate.o \
./src/paleo_latitude/PLPlates.o \
./src/paleo_latitude/PLPolarWanderPaths.o \
./src/paleo_latitude/PaleoLatitude.o 

CPP_DEPS += \
./src/paleo_latitude/PLEulerPolesReconstructions.d \
./src/paleo_latitude/PLParameters.d \
./src/paleo_latitude/PLPlate.d \
./src/paleo_latitude/PLPlates.d \
./src/paleo_latitude/PLPolarWanderPaths.d \
./src/paleo_latitude/PaleoLatitude.d 


# Each subdirectory must supply rules for building sources it contributes
src/paleo_latitude/%.o: ../src/paleo_latitude/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -std=gnu++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


